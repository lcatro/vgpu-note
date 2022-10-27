
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


int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);
int LLVMFuzzerInitialize(int *argc, char ***argv);

#ifndef CLEANUP_EACH_INPUT
// eglInitialize leaks unless eglTeriminate is called (which only happens
// with CLEANUP_EACH_INPUT), so suppress leak detection on everything
// allocated by it.
const char* __lsan_default_suppressions(void);
const char* __lsan_default_suppressions() {
   return "leak:eglInitialize\n";
}

#endif // !CLEANUP_EACH_INPUT

struct fuzzer_cookie
{
   EGLDisplay display;
   EGLConfig egl_config;
   EGLContext ctx;
};

static void fuzzer_write_fence(void *opaque, uint32_t fence)
{
}

static virgl_renderer_gl_context fuzzer_create_gl_context(
      void *cookie, int scanout_idx, struct virgl_renderer_gl_ctx_param *param)
{
   struct fuzzer_cookie *cookie_data = cookie;
   EGLContext shared = param->shared ? eglGetCurrentContext() : NULL;
   const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3,
                                      EGL_NONE };
   EGLContext ctx = eglCreateContext(cookie_data->display,
                                     cookie_data->egl_config,
                                     shared,
                                     context_attribs);
   assert(ctx);

   return ctx;
}

static void fuzzer_destroy_gl_context(void *cookie,
                                      virgl_renderer_gl_context ctx)
{
   struct fuzzer_cookie *cookie_data = cookie;
   eglDestroyContext(cookie_data->display, ctx);
}

static int fuzzer_make_current(void *cookie, int scanout_idx, virgl_renderer_gl_context ctx)
{
   return 0;
}

const int FUZZER_CTX_ID = 1;
const char *SWRAST_ENV = "LIBGL_ALWAYS_SOFTWARE";

static struct fuzzer_cookie cookie;

static struct virgl_renderer_callbacks fuzzer_cbs = {
   .version = 1,
   .write_fence = fuzzer_write_fence,
   .create_gl_context = fuzzer_create_gl_context,
   .destroy_gl_context = fuzzer_destroy_gl_context,
   .make_current = fuzzer_make_current,
};

static bool initialized = false;

static int initialize_environment()
{
   if (!initialized) {
      EGLBoolean ok;

      // Force SW rendering unless env variable is already set.
      setenv(SWRAST_ENV, "true", 0);

      cookie.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      assert(cookie.display != EGL_NO_DISPLAY);

      ok = eglInitialize(cookie.display, NULL, NULL);
      assert(ok);

      const EGLint config_attribs[] = { EGL_SURFACE_TYPE, EGL_DONT_CARE,
                                        EGL_NONE };
      EGLint num_configs;
      ok = eglChooseConfig(cookie.display, config_attribs,
                           &cookie.egl_config, 1, &num_configs);
      assert(ok);

      ok = eglBindAPI(EGL_OPENGL_ES_API);
      assert(ok);

      const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3,
                                         EGL_NONE };
      cookie.ctx = eglCreateContext(cookie.display, cookie.egl_config,
                                    EGL_NO_CONTEXT, context_attribs);
      assert(cookie.ctx != EGL_NO_CONTEXT);

      ok = eglMakeCurrent(cookie.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                          cookie.ctx);
      assert(ok);

      initialized = true;
   }

   return FUZZER_CTX_ID;
}

#ifdef CLEANUP_EACH_INPUT
static void cleanup_environment()
{
   if (cookie.ctx != EGL_NO_CONTEXT) {
      eglMakeCurrent(cookie.display, NULL, NULL, NULL);
      eglDestroyContext(cookie.display, cookie.ctx);
   }

   if (cookie.display != EGL_NO_DISPLAY) {
      eglTerminate(cookie.display);
   }

   initialized = false;
}
#endif

uint32_t ctx_id = 1;

int LLVMFuzzerInitialize(int *argc, char ***argv) {
   initialize_environment();

   int ret = virgl_renderer_init(&cookie, 0, &fuzzer_cbs);
   assert(!ret);

   return 0;
}

#define PIPE_SHADER_COMPUTE  5
#define PIPE_SHADER_TYPES    6

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /*
    if (size < 10)
        return 1;
    if (data[size - 1] != '\0')
        return 1;
        */
    if (size < ((VIRGL_OBJ_SHADER_HDR_SIZE(0) + 4) * 4) || !(size % 4 == 0))
        return 1;

    int ret;
    const char *name = "fuzzctx";
    ret = virgl_renderer_context_create(ctx_id, strlen(name), name);
    assert(!ret);

    int res_id = 1;

    
    uint32_t cmd_buffer_size = size + 12;
    uint32_t cmd[4096] = {0};
    int i = 0;
    cmd[i++] = size / 4 << 16 | VIRGL_OBJECT_SHADER << 8 | VIRGL_CCMD_CREATE_OBJECT;
    cmd[i++] = res_id;  //  handle
    cmd[i++] = PIPE_SHADER_COMPUTE;//(data[0] % (PIPE_SHADER_TYPES));//  type <= PIPE_SHADER_COMPUTE
    memcpy(&cmd[i],data,size);

    virgl_renderer_submit_cmd((void *) cmd, ctx_id, cmd_buffer_size / 4);
    
/*
    uint32_t real_size = size + 10;

    struct tgsi_token *tokens = calloc(real_size, sizeof(struct tgsi_token));
    tgsi_text_translate((const char *)data, tokens, real_size);

    virgl_renderer_context_destroy(ctx_id);

    free(tokens);
*/

    return 0;
}
