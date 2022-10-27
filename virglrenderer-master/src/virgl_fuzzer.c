// Copyright 2018 The Chromium OS Authors. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// libfuzzer-based fuzzer for public APIs.

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

uint32_t ctx_id = 0;

int LLVMFuzzerInitialize(int *argc, char ***argv) {
   ctx_id = initialize_environment();

   int ret = virgl_renderer_init(&cookie, 0, &fuzzer_cbs);
   assert(!ret);

   return 0;
}

#define POWER_SWITCH_ON_COMMAND_SINGAL          (0)
#define POWER_SWITCH_ON_COMMAND_MUTIL_NO_CHANGE (1 << 0)
#define POWER_SWITCH_ON_COMMAND_MUTIL_CHANGE    (1 << 1)
#define POWER_SWITCH_ON_RESOURCE_SINGAL         (1 << 4)
#define POWER_SWITCH_ON_RESOURCE_MUTIL          (1 << 5)
#define POWER_SWITCH_ON_GET_FENCING_FD          (1 << 8)
#define POWER_SWITCH_ON_RETIRE_FENCING          (1 << 9)
#define POWER_SWITCH_ON_SUBMIT_FENCING          (1 << 10)


int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   if (size < 0x200 || (size % 4) == 0)
      return 1;

   int ret;

   // There are trade-offs here between ensuring that state is not persisted
   // between invocations of virgl_renderer_submit_cmd, and to avoid leaking
   // resources that comes with repeated dlopen()/dlclose()ing the mesa
   // driver with each eglInitialize()/eglTerminate() if CLEANUP_EACH_INPUT
   // is set.

   const char *name = "fuzzctx";
   ret = virgl_renderer_context_create(ctx_id, strlen(name), name);
   assert(!ret);

   struct virgl_renderer_resource_create_args args;
   int index = 0;
   int arg_count = 0;
   uint32_t arg_list[50] = {0};
   uint8_t flag = size >> 3;

   if (flag & POWER_SWITCH_ON_RESOURCE_SINGAL) {
      uint32_t* point = (uint32_t*)data;
      args.handle = *point++;
      args.target = *point++;
      args.format = *point++;
      args.bind = *point++;
      args.width = *point++;
      args.height = *point++;
      args.depth = *point++;
      args.array_size = *point++;
      args.last_level = *point++;
      args.nr_samples = *point++;
      args.flags = *point++;

      virgl_renderer_resource_create(&args, NULL, 0);
      virgl_renderer_ctx_attach_resource(ctx_id, args.handle);
   } else if (flag & POWER_SWITCH_ON_RESOURCE_MUTIL) {
      for (index = 0;index < 4;++index) {
         int offset = sizeof(struct virgl_renderer_resource_create_args) * index;
         args.handle = *(uint32_t*)&data[offset + 0];
         args.target = *(uint32_t*)&data[offset + 4];
         args.format = *(uint32_t*)&data[offset + 8];
         args.bind = *(uint32_t*)&data[offset + 12];
         args.width = *(uint32_t*)&data[offset + 16];
         args.height = *(uint32_t*)&data[offset + 20];
         args.depth = *(uint32_t*)&data[offset + 24];
         args.array_size = *(uint32_t*)&data[offset + 28];
         args.last_level = *(uint32_t*)&data[offset + 32];
         args.nr_samples = *(uint32_t*)&data[offset + 36];
         args.flags = *(uint32_t*)&data[offset + 40];

         virgl_renderer_resource_create(&args, NULL, 0);
         virgl_renderer_ctx_attach_resource(ctx_id, args.handle);

         arg_list[index] = args.handle;
         arg_count = index;
      }
   }

   unsigned int new_size = size + 4;
   uint32_t* command_buffer = (unsigned char*)malloc(new_size);

   if ((flag & POWER_SWITCH_ON_COMMAND_SINGAL)) {  //  单个cmd测试
      memset(command_buffer,0,new_size);
      command_buffer[0] = (size / 4) - 1 << 16 | data[0] % 0xFF << 8 | data[1] % VIRGL_MAX_COMMANDS;
      memcpy(&command_buffer[1],&data[2],size - 2);

      virgl_renderer_submit_cmd((void *) command_buffer, ctx_id, (size + 1) / 4);
   } else if ((flag & POWER_SWITCH_ON_COMMAND_MUTIL_NO_CHANGE)) {  //  cmd改变但是data不改变
      int loop_count = size % 0x10;

      for (index = 0;index < loop_count;++index) {
         memset(command_buffer,0,new_size);
         command_buffer[0] = (size / 4) - 1 << 16 | data[0] % 0xFF << 8 | data[1] % VIRGL_MAX_COMMANDS;
         memcpy(&command_buffer[1],&data[2],size - 2);

         virgl_renderer_submit_cmd((void *) command_buffer, ctx_id, (size + 1) / 4);
      }
   } else {  //  cmd,data都改变
      int loop_count = size % 0x10;
      int next_data_offset = 0;

      for (int index = 0;index < loop_count;++index) {
         int temp_size = data[next_data_offset + 1];

         if (temp_size < 4 || next_data_offset + temp_size + 2 >= size)
            break;

         memset(command_buffer,0,new_size);
         command_buffer[0] = (size / 4) - 1 << 16 | data[0] % 0xFF << 8 | data[1] % VIRGL_MAX_COMMANDS;
         memcpy(&command_buffer[1],&data[next_data_offset + 3],temp_size - 2);

         virgl_renderer_submit_cmd((void *) command_buffer, ctx_id, temp_size / 4);

         next_data_offset += temp_size;
      }
   }

   if (flag & POWER_SWITCH_ON_RESOURCE_SINGAL) {
      virgl_renderer_ctx_detach_resource(ctx_id, args.handle);
   } else if (flag & POWER_SWITCH_ON_RESOURCE_MUTIL) {
      for (index = 0;index < arg_count;++index)
         virgl_renderer_ctx_detach_resource(ctx_id, arg_list[index]);
   }

   virgl_renderer_context_destroy(ctx_id);
   ///virgl_resource_table_reset();   为什么不要reset,是因为reset会让所有的handle失效.
   ///如果handle和virgl_renderer_submit_cmd里面的不一致,那就会导致后面的命令找不到资源id

   free(command_buffer);

//   virgl_renderer_cleanup(&cookie);

#ifdef CLEANUP_EACH_INPUT
   // The following cleans up between each input which is a lot slower.
   //cleanup_environment();
#endif

   return 0;
}
