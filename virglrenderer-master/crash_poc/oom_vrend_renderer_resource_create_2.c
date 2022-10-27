
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
                      VIRGL_RENDERER_USE_SURFACELESS|VREND_USE_EXTERNAL_BLOB, &fuzzer_cbs);

  int ctx_id = 1;
  int res_id = 2;
  const char *name = "fuzzctx";
  int ret = virgl_renderer_context_create(ctx_id, strlen(name), name);
  struct virgl_renderer_resource_create_args args;
  args.handle = res_id;
  args.target = 0;
  args.format = 10;
  args.bind = 0;//VIRGL_BIND_CUSTOM;
  args.width = 0xFFFFFFFF;
  args.height = 1;
  args.depth = 1;
  args.array_size = 0;
  args.last_level = 0;
  args.nr_samples = 0;
  args.flags = VIRGL_RESOURCE_FLAG_MAP_PERSISTENT;

  virgl_renderer_resource_create(&args, NULL, 0);
  
  return 1;
}

/*

Code:
   if (buffer_storage_flags) {
      if (has_feature(feat_arb_buffer_storage) && !vrend_state.use_external_blob) {
         glBufferStorage(gr->target, width, NULL, buffer_storage_flags);

==121218== ERROR: libFuzzer: out-of-memory (malloc(4228448304))
   To change the out-of-memory limit use -rss_limit_mb=<N>
    #12 0x5a2509 in vrend_create_buffer /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:6933:        glBufferStorage
    #13 0x5a2509 in vrend_resource_alloc_buffer /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:7031:7
    #14 0x5a2509 in vrend_renderer_resource_create /home/fuzzing/Desktop/virglrenderer-master/build/../src/vrend_renderer.c:7336:13
    #15 0x627b50 in virgl_renderer_resource_create_internal /home/fuzzing/Desktop/virglrenderer-master/build/../src/virglrenderer.c:93:15
    #16 0x5536f1 in LLVMFuzzerTestOneInput /home/fuzzing/Desktop/virglrenderer-master/build/../src/virgl_fuzzer.c:241:10
    #17 0x45d861 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long) (/home/fuzzing/Desktop/virglrenderer-master/build/src/virgl_fuzzer+0x45d861)
    #18 0x448fd2 in fuzzer::RunOneTest(fuzzer::Fuzzer*, char const*, unsigned long) (/home/fuzzing/Desktop/virglrenderer-master/build/src/virgl_fuzzer+0x448fd2)
    #19 0x44ea86 in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long)) (/home/fuzzing/Desktop/virglrenderer-master/build/src/virgl_fuzzer+0x44ea86)
    #20 0x477742 in main (/home/fuzzing/Desktop/virglrenderer-master/build/src/virgl_fuzzer+0x477742)
    #21 0x7fed38e280b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #22 0x42369d in _start (/home/fuzzing/Desktop/virglrenderer-master/build/src/virgl_fuzzer+0x42369d)

SUMMARY: libFuzzer: out-of-memory
*/
