#include <err.h>
#include <stdlib.h>
#include <pciaccess.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/io.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "svga.h"
#include "svga_reg.h"
#include "svga3d_reg.h"

extern SVGADevice gSVGA;

uint32_t
SVGA_ReadReg(uint32_t index)
{
	outl(index, gSVGA.ioBase + SVGA_INDEX_PORT);
	return inl(gSVGA.ioBase + SVGA_VALUE_PORT);
}

void
SVGA_WriteReg(uint32_t index,  uint32_t value)
{
	outl(index, gSVGA.ioBase + SVGA_INDEX_PORT);
	outl(value, gSVGA.ioBase + SVGA_VALUE_PORT);
}

void
VMwareWaitForFB(void)
{
	SVGA_WriteReg(SVGA_REG_CONFIG_DONE, true); // 完成当前一次config
	SVGA_WriteReg(SVGA_REG_SYNC, SVGA_SYNC_GENERIC); 
	while (SVGA_ReadReg(SVGA_REG_BUSY)) {
		log("Looping ------ Wait For FB\n");
	} // 等待sync not busy
}

void
VMwareWriteWordToFIFO(uint32_t value)
{
	// 这个函数是抄的x.org的驱动
	uint32_t *VMwareFIFO = gSVGA.fifoMem;

	if ((VMwareFIFO[SVGA_FIFO_NEXT_CMD] + sizeof(uint32_t) == VMwareFIFO[SVGA_FIFO_STOP]) // SVGA_FIFO_NEXT_CMD看起来指向 CUR OFFSET
			|| (VMwareFIFO[SVGA_FIFO_NEXT_CMD] == VMwareFIFO[SVGA_FIFO_MAX] - sizeof(uint32_t)
				&& VMwareFIFO[SVGA_FIFO_STOP] == VMwareFIFO[SVGA_FIFO_MIN])) {

        log("Syncing because of full fifo\n");
		VMwareWaitForFB(); // 等待fifo队列空闲
	}

	VMwareFIFO[VMwareFIFO[SVGA_FIFO_NEXT_CMD] / sizeof(uint32_t)] = value; // fifo[cur_index] = value

	if(VMwareFIFO[SVGA_FIFO_NEXT_CMD] == VMwareFIFO[SVGA_FIFO_MAX] - sizeof(uint32_t)) {
		VMwareFIFO[SVGA_FIFO_NEXT_CMD] = VMwareFIFO[SVGA_FIFO_MIN]; // 当他满了之后, 就将next设置为min, 类似环形队列?
	} else {
		VMwareFIFO[SVGA_FIFO_NEXT_CMD] += sizeof(uint32_t); // index递增
	}
}

void
pci_cleanup(struct pci_device_iterator *iter)
{

	pci_iterator_destroy(iter);
	pci_system_cleanup();
}


int
conf_svga_device(void)
{

	struct pci_device *dev;
	struct pci_device_iterator *iter;
	struct pci_id_match match;
	uint16_t command;

	if (getuid() != 0 || geteuid() != 0) 
		errx(EXIT_FAILURE, "[!] Run program as root");

    if (iopl(3) !=0 ) // 初始化pio
	{
        die("iopl");
	}

	if (pci_system_init()) // 初始化对PCI系统的访问
		return -1;

	match.vendor_id = PCI_VENDOR_ID_VMWARE;
	match.device_id = PCI_DEVICE_ID_VMWARE_SVGA2; // 这个是 SVGA 2 的信息
	match.subvendor_id = PCI_MATCH_ANY;
	match.subdevice_id = PCI_MATCH_ANY;
	match.device_class = 0;
	match.device_class_mask = 0; // 直接通过这个结构去全局搜索

	iter = pci_id_match_iterator_create(&match);
	dev = pci_device_next(iter);

	if (dev == NULL) {
		pci_cleanup(iter);
		return -1;
	}

	// 找到对应设备 pci_device *
	// 一条bus上挂着许多device，而device需要driver才能工作。内核有注册device和注册driver的概念。
	// 		注册device只是将deivce添加到内核中，该设备还不能工作。
	// 		而注册driver，就是在对应的bus上找到device，从而调用driver的probe函数进行初始化，而后继续其它的事情。
	pci_device_probe(dev);

	gSVGA.ioBase = dev->regions[0].base_addr; // ioport
	gSVGA.fbMem = (void *)dev->regions[1].base_addr; // framebuffer memory
	gSVGA.fifoMem = (void *)dev->regions[2].base_addr; // FIFO

	command = pci_device_cfg_read_u16(dev, 0, 4);
	pci_device_cfg_write_u16(dev, command | 7, 4); // 这个是直接去修改CONFIG

	SVGA_WriteReg(SVGA_REG_ID, SVGA_ID_2); // 这里是使用 SVGA v2 版本
	SVGA_WriteReg(SVGA_REG_ENABLE, true); // 启用reg

	gSVGA.vramSize = SVGA_ReadReg(SVGA_REG_VRAM_SIZE);
	gSVGA.fbSize = SVGA_ReadReg(SVGA_REG_FB_SIZE);
	gSVGA.fifoSize = SVGA_ReadReg(SVGA_REG_MEM_SIZE);

	log("gSVGA.vramSize = 0x%x\n", gSVGA.vramSize);
	log("gSVGA.fbSize = 0x%x\n", gSVGA.fbSize);
	log("gSVGA.fifoSize = 0x%x\n", gSVGA.fifoSize);

	// remap real address to user space
	int bar1_map = pci_device_map_range(dev, (pciaddr_t)gSVGA.fbMem, (pciaddr_t)gSVGA.fbSize,
			PCI_DEV_MAP_FLAG_WRITABLE, (void *)&gSVGA.fbMem);
	int bar2_map = pci_device_map_range(dev, (pciaddr_t)gSVGA.fifoMem, (pciaddr_t)gSVGA.fifoSize,
			PCI_DEV_MAP_FLAG_WRITABLE, (void *)&gSVGA.fifoMem);

	log("bar1_map = 0x%x\n", bar1_map);
	log("bar2_map = 0x%x\n", bar2_map);


	pci_cleanup(iter);

	return 0;
}



void hexdump(const char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);
    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);
            // Output the offset.
            printf("  %04x ", i);
        }
        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);
        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

#define PAGE_SHIFT  12
#define PAGE_SIZE   (1 << PAGE_SHIFT)
#define PFN_PRESENT (1ull << 63)
#define PFN_PFN     ((1ull << 55) - 1)


int fd;
uint32_t page_offset(uint32_t addr)
{
    return addr & ((1 << PAGE_SHIFT) - 1);
}

uint64_t gva_to_gfn(void *addr)
{
    uint64_t pme, gfn;
    size_t offset;
    offset = ((uintptr_t)addr >> 9) & ~7;
    lseek(fd, offset, SEEK_SET);
    read(fd, &pme, 8);
    if (!(pme & PFN_PRESENT))
        return -1;
    gfn = pme & PFN_PFN;
    return gfn;
}

uint64_t gva_to_gpa(void *addr)
{
	fd = open("/proc/self/pagemap", O_RDONLY);
    uint64_t gfn = gva_to_gfn(addr);
    assert(gfn != -1);
	close(fd);
    return (gfn << PAGE_SHIFT) | page_offset((uint64_t)addr);
}


PhyMem *dma_alloc(){
	PhyMem * ret = malloc(sizeof(*ret));
	ret->vptr = mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,0,0);
	memset(ret->vptr, 0, 0x1000);
	ret->phyaddr = gva_to_gpa(ret->vptr);
	log("dma alloc: vptr=%p phyaddr=0x%lx size=0x%x\n", ret->vptr, ret->phyaddr, 0x1000);

	return ret;
}





int
SVGA_HasFIFOCap(int cap)
{
   return (gSVGA.fifoMem[SVGA_FIFO_CAPABILITIES] & cap) != 0;
}


int
SVGA_GetFIFOCap()
{
   return gSVGA.fifoMem[SVGA_FIFO_CAPABILITIES];
}

int
SVGA_IsFIFORegValid(int reg)
{
   return gSVGA.fifoMem[SVGA_FIFO_MIN] > (reg << 2);
}

void
SVGA_RingDoorbell(void)
{
   if (SVGA_IsFIFORegValid(SVGA_FIFO_BUSY) &&
       gSVGA.fifoMem[SVGA_FIFO_BUSY] == 0) {

      /* Remember that we already rang the doorbell. */
      gSVGA.fifoMem[SVGA_FIFO_BUSY] = 1;

      /*
       * Asynchronously wake up the SVGA3D device.  The second
       * parameter is an arbitrary nonzero 'sync reason' which can be
       * used for debugging, but which isn't part of the SVGA3D
       * protocol proper and which isn't used by release builds of
       * VMware products.
       */
      SVGA_WriteReg(SVGA_REG_SYNC, 1);
   }
}

void
SVGAFIFOFull(void)
{
   if (SVGA_IsFIFORegValid(SVGA_FIFO_FENCE_GOAL) &&
       (gSVGA.capabilities & SVGA_CAP_IRQMASK)) {

      /*
       * On hosts which support interrupts, we can sleep until the
       * FIFO_PROGRESS interrupt occurs. This is the most efficient
       * thing to do when the FIFO fills up.
       *
       * As with the IRQ-based SVGA_SyncToFence(), this will only work
       * on Workstation 6.5 virtual machines and later.
       */

      SVGA_WriteReg(SVGA_REG_IRQMASK, SVGA_IRQFLAG_FIFO_PROGRESS);
      SVGA_RingDoorbell();
      SVGA_WriteReg(SVGA_REG_IRQMASK, 0);

   } else {

      /*
       * Fallback implementation: Perform one iteration of the
       * legacy-style sync. This synchronously processes FIFO commands
       * for an arbitrary amount of time, then returns control back to
       * the guest CPU.
       */

      SVGA_WriteReg(SVGA_REG_SYNC, 1);
      SVGA_ReadReg(SVGA_REG_BUSY);
   }
}


void *
SVGA_FIFOReserve(uint32 bytes)  // IN
{
   volatile uint32 *fifo = gSVGA.fifoMem;
   uint32 max = fifo[SVGA_FIFO_MAX];
   uint32 min = fifo[SVGA_FIFO_MIN];
   uint32 nextCmd = fifo[SVGA_FIFO_NEXT_CMD];
   int reserveable = SVGA_HasFIFOCap(SVGA_FIFO_CAP_RESERVE);

   /*
    * This example implementation uses only a statically allocated
    * buffer.  If you want to support arbitrarily large commands,
    * dynamically allocate a buffer if and only if it's necessary.
    */

   if (bytes > sizeof gSVGA.fifo.bounceBuffer ||
       bytes > (max - min)) {
      printf("FIFO command too large");
   }

   if (bytes % sizeof(uint32)) {
      printf("FIFO command length not 32-bit aligned");
   }

   if (gSVGA.fifo.reservedSize != 0) {
      printf("FIFOReserve before FIFOCommit");
   }

   gSVGA.fifo.reservedSize = bytes;

   while (1) {
      uint32 stop = fifo[SVGA_FIFO_STOP];
      int reserveInPlace = 0;
      int needBounce = 0;

      /*
       * Find a strategy for dealing with "bytes" of data:
       * - reserve in place, if there's room and the FIFO supports it
       * - reserve in bounce buffer, if there's room in FIFO but not
       *   contiguous or FIFO can't safely handle reservations
       * - otherwise, sync the FIFO and try again.
       */

      if (nextCmd >= stop) {
         /* There is no valid FIFO data between nextCmd and max */

         if (nextCmd + bytes < max ||
             (nextCmd + bytes == max && stop > min)) {
            /*
             * Fastest path 1: There is already enough contiguous space
             * between nextCmd and max (the end of the buffer).
             *
             * Note the edge case: If the "<" path succeeds, we can
             * quickly return without performing any other tests. If
             * we end up on the "==" path, we're writing exactly up to
             * the top of the FIFO and we still need to make sure that
             * there is at least one unused DWORD at the bottom, in
             * order to be sure we don't fill the FIFO entirely.
             *
             * If the "==" test succeeds, but stop <= min (the FIFO
             * would be completely full if we were to reserve this
             * much space) we'll end up hitting the FIFOFull path below.
             */
            reserveInPlace = 1;
         } else if ((max - nextCmd) + (stop - min) <= bytes) {
            /*
             * We have to split the FIFO command into two pieces,
             * but there still isn't enough total free space in
             * the FIFO to store it.
             *
             * Note the "<=". We need to keep at least one DWORD
             * of the FIFO free at all times, or we won't be able
             * to tell the difference between full and empty.
             */
            SVGAFIFOFull();
         } else {
            /*
             * Data fits in FIFO but only if we split it.
             * Need to bounce to guarantee contiguous buffer.
             */
            needBounce = 1;
         }

      } else {
         /* There is FIFO data between nextCmd and max */

         if (nextCmd + bytes < stop) {
            /*
             * Fastest path 2: There is already enough contiguous space
             * between nextCmd and stop.
             */
            reserveInPlace = 1;
         } else {
            /*
             * There isn't enough room between nextCmd and stop.
             * The FIFO is too full to accept this command.
             */
            SVGAFIFOFull();
         }
      }

      /*
       * If we decided we can write directly to the FIFO, make sure
       * the VMX can safely support this.
       */
      if (reserveInPlace) {
         if (reserveable || bytes <= sizeof(uint32)) {
            gSVGA.fifo.usingBounceBuffer = 0;
            if (reserveable) {
               fifo[SVGA_FIFO_RESERVED] = bytes;
            }
            return nextCmd + (uint8*) fifo;
         } else {
            /*
             * Need to bounce because we can't trust the VMX to safely
             * handle uncommitted data in FIFO.
             */
            needBounce = 1;
         }
      }

      /*
       * If we reach here, either we found a full FIFO, called
       * SVGAFIFOFull to make more room, and want to try again, or we
       * decided to use a bounce buffer instead.
       */
      if (needBounce) {
         gSVGA.fifo.usingBounceBuffer = 1;
         return gSVGA.fifo.bounceBuffer;
      }
   } /* while (1) */
}


/*
 *-----------------------------------------------------------------------------
 *
 * SVGA_FIFOCommit --
 *
 *      Commit a block of FIFO data which was placed in the buffer
 *      returned by SVGA_FIFOReserve. Every Reserve must be paired
 *      with exactly one Commit, but the sizes don't have to match.
 *      The caller is free to commit less space than they
 *      reserved. This can be used if the command size isn't known in
 *      advance, but it is reasonable to make a worst-case estimate.
 *
 *      The commit size does not have to match the size of a single
 *      FIFO command. This can be used to write a partial command, or
 *      to write multiple commands at once.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */



void
SVGA_FIFOCommit(uint32 bytes)  // IN
{
   volatile uint32 *fifo = gSVGA.fifoMem;
   uint32 nextCmd = fifo[SVGA_FIFO_NEXT_CMD];
   uint32 max = fifo[SVGA_FIFO_MAX];
   uint32 min = fifo[SVGA_FIFO_MIN];
   int reserveable = SVGA_HasFIFOCap(SVGA_FIFO_CAP_RESERVE);

   if (gSVGA.fifo.reservedSize == 0) {
      printf("FIFOCommit before FIFOReserve");
   }
   gSVGA.fifo.reservedSize = 0;

   if (gSVGA.fifo.usingBounceBuffer) {
      /*
       * Slow paths: copy out of a bounce buffer.
       */
      uint8 *buffer = gSVGA.fifo.bounceBuffer;

      if (reserveable) {
         /*
          * Slow path: bulk copy out of a bounce buffer in two chunks.
          *
          * Note that the second chunk may be zero-length if the reserved
          * size was large enough to wrap around but the commit size was
          * small enough that everything fit contiguously into the FIFO.
          *
          * Note also that we didn't need to tell the FIFO about the
          * reservation in the bounce buffer, but we do need to tell it
          * about the data we're bouncing from there into the FIFO.
          */

         uint32 chunkSize = 0;
		 if (bytes > max - nextCmd)
		 	chunkSize = max - nextCmd;
		else
			chunkSize = bytes;
			
         fifo[SVGA_FIFO_RESERVED] = bytes;
         memcpy(nextCmd + (uint8*) fifo, buffer, chunkSize);
         memcpy(min + (uint8*) fifo, buffer + chunkSize, bytes - chunkSize);

      } else {
         /*
          * Slowest path: copy one dword at a time, updating NEXT_CMD as
          * we go, so that we bound how much data the guest has written
          * and the host doesn't know to checkpoint.
          */

         uint32 *dword = (uint32 *)buffer;

         while (bytes > 0) {
            fifo[nextCmd / sizeof *dword] = *dword++;
            nextCmd += sizeof *dword;
            if (nextCmd == max) {
               nextCmd = min;
            }
            fifo[SVGA_FIFO_NEXT_CMD] = nextCmd;
            bytes -= sizeof *dword;
         }
      }
   }

   /*
    * Atomically update NEXT_CMD, if we didn't already
    */
   if (!gSVGA.fifo.usingBounceBuffer || reserveable) {
      nextCmd += bytes;
      if (nextCmd >= max) {
         nextCmd -= max - min;
      }
      fifo[SVGA_FIFO_NEXT_CMD] = nextCmd;
   }

   /*
    * Clear the reservation in the FIFO.
    */
   if (reserveable) {
      fifo[SVGA_FIFO_RESERVED] = 0;
   }
}


/*
 *-----------------------------------------------------------------------------
 *
 * SVGA_FIFOCommitAll --
 *
 *      This is a convenience wrapper for SVGA_FIFOCommit(), which
 *      always commits the last reserved block in its entirety.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      SVGA_FIFOCommit.
 *
 *-----------------------------------------------------------------------------
 */

void
SVGA_FIFOCommitAll(void)
{
   SVGA_FIFOCommit(gSVGA.fifo.reservedSize);
}


/*
 *-----------------------------------------------------------------------------
 *
 * SVGA_FIFOReserveCmd --
 *
 *      This is a convenience wrapper around SVGA_FIFOReserve, which
 *      prefixes the reserved memory block with a uint32 that
 *      indicates the command type.
 *
 * Results:
 *      Always returns a pointer to 'bytes' bytes of reserved space in the FIFO.
 *
 * Side effects:
 *      Begins a FIFO command, reserves space in the FIFO. Writes a
 *      1-word header into the FIFO.  May block (in SVGAFIFOFull) if
 *      the FIFO is full.
 *
 *-----------------------------------------------------------------------------
 */

void *
SVGA_FIFOReserveCmd(uint32 type,   // IN
                    uint32 bytes)  // IN
{
   uint32 *cmd = SVGA_FIFOReserve(bytes + sizeof type);
   cmd[0] = type;
   return cmd + 1;
}


/*
 *-----------------------------------------------------------------------------
 *
 * SVGA_FIFOReserveEscape --
 *
 *      This is a convenience wrapper around SVGA_FIFOReserve, which
 *      prefixes the reserved memory block with an ESCAPE command header.
 *
 *      ESCAPE commands are a way of encoding extensible and
 *      variable-length packets within the basic FIFO protocol
 *      itself. ESCAPEs are used for some SVGA device functionality,
 *      like video overlays, for VMware's internal debugging tools,
 *      and for communicating with third party code that can load into
 *      the SVGA device.
 *
 * Results:
 *      Always returns a pointer to 'bytes' bytes of reserved space in the FIFO.
 *
 * Side effects:
 *      Begins a FIFO command, reserves space in the FIFO. Writes a
 *      3-word header into the FIFO.  May block (in SVGAFIFOFull) if
 *      the FIFO is full.
 *
 *-----------------------------------------------------------------------------
 */

void *
SVGA_FIFOReserveEscape(uint32 nsid,   // IN
                       uint32 bytes)  // IN
{
   uint32 paddedBytes = (bytes + 3) & ~3UL;
   struct {
      uint32 cmd;
      uint32 nsid;
      uint32 size;
   } __attribute__ ((__packed__)) *header = SVGA_FIFOReserve(paddedBytes
                                                             + sizeof *header);

   header->cmd = SVGA_CMD_ESCAPE;
   header->nsid = nsid;
   header->size = bytes;

   return header + 1;
}



void *
SVGA3D_FIFOReserve(uint32 cmd,      // IN
                   uint32 cmdSize)  // IN
{
   SVGA3dCmdHeader *header;

   header = SVGA_FIFOReserve(sizeof *header + cmdSize);
   header->id = cmd;
   header->size = cmdSize;

   return &header[1];
}



void
SVGA3D_DefineContext(uint32 cid)  // IN
{
   SVGA3dCmdDefineContext *cmd;
   cmd = SVGA3D_FIFOReserve(SVGA_3D_CMD_CONTEXT_DEFINE, sizeof *cmd);
   cmd->cid = cid;
   SVGA_FIFOCommitAll();
}





void
SVGA3D_BeginClear(uint32 cid,             // IN
                  SVGA3dClearFlag flags,  // IN
                  uint32 color,           // IN
                  float depth,            // IN
                  uint32 stencil,         // IN
                  SVGA3dRect **rects,     // OUT
                  uint32 numRects)        // IN
{
   SVGA3dCmdClear *cmd;
   cmd = SVGA3D_FIFOReserve(SVGA_3D_CMD_CLEAR, sizeof *cmd +
                            sizeof **rects * numRects);
   cmd->cid = cid;
   cmd->clearFlag = flags;
   cmd->color = color;
   cmd->depth = depth;
   cmd->stencil = stencil;
   *rects = (SVGA3dRect*) &cmd[1];
   SVGA_FIFOCommitAll();
}







