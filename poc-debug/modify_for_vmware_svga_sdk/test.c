
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svga.h"
#include "svga3d_reg.h"



int main(int argc, char **argv)
{
    SVGA_Init();


	uint32_t cap = SVGA_ReadReg(SVGA_REG_CAPABILITIES);

    
	printf("cap = 0x%x\n", cap); // poc.c:29:main(): cap = 0xfdff83e2, 0xfdffc3e2

    return;
}


