
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

  struct virgl_context *ctx = virgl_context_lookup(ctx_id);
  struct virgl_renderer_resource_create_args args;
  args.handle = res_id;
  args.target = 3;
  args.format = 10;
  args.bind = 10;
  args.width = 2;
  args.height = 1;
  args.depth = 1;
  args.array_size = 0;
  args.last_level = 0;
  args.nr_samples = 0;
  args.flags = 0;

  virgl_renderer_resource_create(&args, NULL, 0);
  virgl_renderer_ctx_attach_resource(ctx_id, args.handle);
  
  //vrend_renderer_get_meminfo(ctx,res_id);
  uint32_t cmd[2] = {0};
  int i = 0;
  cmd[i++] = 1 << 16 | VIRGL_CCMD_GET_MEMORY_INFO;
  cmd[i++] = 0x00000001;  //  handle  <<<<<

  virgl_renderer_submit_cmd((void *) cmd, ctx_id, 2);

  return 1;
}

/*
AddressSanitizer:DEADLYSIGNAL
=================================================================
==140734==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x00000052247b bp 0x7ffcbbeee8e0 sp 0x7ffcbbeee800 T0)
==140734==The signal is caused by a READ memory access.
==140734==Hint: address points to the zero page.
    #0 0x52247b in vrend_renderer_get_meminfo /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:11480:49
    #1 0x4d9129 in vrend_decode_get_memory_info /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:1570:4
    #2 0x4cfdbd in vrend_decode_ctx_submit_cmd /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_decode.c:1706:13
    #3 0x4c9608 in main /home/fuzzing/Desktop/virglrenderer-master/build/../src/crash_vrend_renderer_get_meminfo.c:97:3
    #4 0x7f2aaab570b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #5 0x4215ad in _start (/home/fuzzing/Desktop/virglrenderer-master/build/src/crash_vrend_renderer_get_meminfo+0x4215ad)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:11480:49 in vrend_renderer_get_meminfo
==117292==ABORTING

*/
