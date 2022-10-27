#include <stdint.h>
#include <stdbool.h>

typedef long long int64;
typedef unsigned long long uint64;

typedef int int32;
typedef unsigned int uint32;

typedef short int16;
typedef unsigned short uint16;

typedef char int8;
typedef unsigned char uint8;

typedef uint8 Bool;


#define FALSE false
#define TRUE true

#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

typedef struct SVGADevice {
	uint32     ioBase;
	uint32    *fifoMem;
	uint8     *fbMem;
	uint32     fifoSize;
	uint32     fbSize;
	uint32     vramSize;

	uint32     deviceVersionId;
	uint32     capabilities;

	uint32     width;
	uint32     height;
	uint32     bpp;
	uint32     pitch;

	struct {
		uint32  reservedSize;
		Bool    usingBounceBuffer;
		uint8   bounceBuffer[1024 * 1024];
		uint32  nextFence;
	} fifo;

} SVGADevice;

uint32_t SVGA_ReadReg(uint32_t);
void SVGA_WriteReg(uint32_t,  uint32_t);
void VMwareWaitForFB(void);
void VMwareWriteWordToFIFO(uint32_t);
int conf_svga_device(void);


// 下面都是个人新增的
#include <stdio.h>

#define log(fmt, ...)                                                   \
    do {                                                                       \
        fprintf(stderr, "%s:%d:%s(): " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
    } while (0)

#define pause() \
    do{ \
        fprintf(stderr, "%s:%d:%s(): pause\n", __FILE__, __LINE__, __func__); \
        getchar(); \
    }while(0)


#include <inttypes.h>
#define u8 uint8_t
#define s8 int8_t
#define u16 uint16_t
#define s16 int16_t
#define u32 uint32_t
#define s32 int32_t
#define u64 uint64_t
#define s64 int64_t

void hexdump(const char *desc, void *addr, int len);
#define die(fmt, ...)                                                   \
    do {                                                                       \
        fprintf(stderr, "%s:%d:%s(): " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
		exit(-1); \
	} while (0)

typedef struct PhyMem{
	void *vptr; 
	u64 phyaddr;
} PhyMem;

// 用来在r3直接分配和使用r0内存
PhyMem *dma_alloc();

#define PA2PPN(pa) (pa>>0xc)

// 尝试修复的库
void
SVGA_FIFOCommit(uint32 bytes) ;
void
SVGA_FIFOCommitAll(void);
void *
SVGA_FIFOReserve(uint32 bytes);
void
SVGA_SetMode(uint32 width,   // IN
             uint32 height,  // IN
             uint32 bpp);     // IN

void
SVGA3D_Init(void);