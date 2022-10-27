#############################################################################
#
# Copyright (C) 2019 Collabora Ltd
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#

subdir('mesa')
subdir('gallium')

virgl_sources = [
   'iov.c',
   'virgl_context.c',
   'virgl_context.h',
   'virgl_hw.h',
   'virgl_protocol.h',
   'virgl_resource.c',
   'virgl_resource.h',
   'virgl_util.c',
   'virgl_util.h',
]

vrend_sources = [
   'vrend_blitter.c',
   'vrend_blitter.h',
   'vrend_debug.c',
   'vrend_debug.h',
   'vrend_decode.c',
   'vrend_formats.c',
   'vrend_iov.h',
   'vrend_object.c',
   'vrend_object.h',
   'vrend_renderer.c',
   'vrend_renderer.h',
   'vrend_shader.c',
   'vrend_shader.h',
   'vrend_strbuf.h',
   'vrend_tweaks.c',
   'vrend_tweaks.h',
   'vrend_winsys.c',
   'vrend_winsys.h',
]

virglrenderer_sources = [
   'virglrenderer.c',
   'virglrenderer.h',
   'virglrenderer_hw.h',
]

vrend_winsys_egl_sources = [
   'vrend_winsys_gbm.c',
   'vrend_winsys_gbm.h',
   'vrend_winsys_egl.c',
   'vrend_winsys_egl.h',
]

vrend_winsys_glx_sources = [
   'vrend_winsys_glx.c',
   'vrend_winsys_glx.h',
]

venus_sources = [
   'venus_hw.h',
   'venus/venus-protocol/vn_protocol_renderer.h',
   'venus/vkr_buffer.c',
   'venus/vkr_buffer.h',
   'venus/vkr_command_buffer.c',
   'venus/vkr_command_buffer.h',
   'venus/vkr_common.c',
   'venus/vkr_common.h',
   'venus/vkr_context.c',
   'venus/vkr_context.h',
   'venus/vkr_cs.c',
   'venus/vkr_cs.h',
   'venus/vkr_descriptor_set.c',
   'venus/vkr_descriptor_set.h',
   'venus/vkr_device.c',
   'venus/vkr_device.h',
   'venus/vkr_device_memory.c',
   'venus/vkr_device_memory.h',
   'venus/vkr_image.c',
   'venus/vkr_image.h',
   'venus/vkr_instance.c',
   'venus/vkr_instance.h',
   'venus/vkr_physical_device.c',
   'venus/vkr_physical_device.h',
   'venus/vkr_pipeline.c',
   'venus/vkr_pipeline.h',
   'venus/vkr_query_pool.c',
   'venus/vkr_query_pool.h',
   'venus/vkr_queue.c',
   'venus/vkr_queue.h',
   'venus/vkr_render_pass.c',
   'venus/vkr_render_pass.h',
   'venus/vkr_renderer.c',
   'venus/vkr_renderer.h',
   'venus/vkr_ring.c',
   'venus/vkr_ring.h',
   'venus/vkr_transport.c',
   'venus/vkr_transport.h',
]

venus_codegen = custom_target(
   'venus_codegen',
   input : ['venus/vkr_device_object.py', 'venus/vkr_device_object.json'],
   output : [
      'vkr_buffer_gen.h',
      'vkr_command_buffer_gen.h',
      'vkr_descriptor_set_gen.h',
      'vkr_device_memory_gen.h',
      'vkr_image_gen.h',
      'vkr_pipeline_gen.h',
      'vkr_query_pool_gen.h',
      'vkr_queue_gen.h',
      'vkr_render_pass_gen.h',
   ],
   command : [prog_python, '@INPUT0@', '-o', '@OUTDIR@', '@INPUT1@'],
)

virgl_depends = [
   gallium_dep,
   epoxy_dep,
   libdrm_dep,
   thread_dep,
   m_dep,
]

if with_tracing == 'perfetto'
   virgl_depends += [vperfetto_min_dep]
endif

if with_tracing == 'percetto'
   virgl_depends += [percetto_dep]
endif

virgl_sources += vrend_sources

if have_egl
   virgl_sources += vrend_winsys_egl_sources
   virgl_depends += [gbm_dep]
endif

if have_glx
   virgl_sources += vrend_winsys_glx_sources
   virgl_depends += [glx_dep]
endif

if with_venus
   virgl_sources += venus_sources
   virgl_sources += venus_codegen
   virgl_depends += [venus_dep]
endif

#libvirgl = executable(
#   'virgl',
#   virgl_sources,
#   include_directories: [inc_gallium, inc_configuration, 'venus'],
#   dependencies : virgl_depends,c_args:['-fsanitize=address','-fsanitize-coverage=trace-cmp,trace-gep,trace-div','-g' ]
#)

libvirgl_inc = [
   inc_gallium,
   inc_configuration,
   include_directories(['.', 'venus'])
]

#libvirgl_dep = declare_dependency(
#   link_with: libvirgl,
#   include_directories: libvirgl_inc
#)

#libvirglrenderer = static_library(
#   'virglrenderer',
#   virglrenderer_sources,
#   include_directories: [inc_gallium, inc_configuration],
#   dependencies : [virgl_depends, libvirgl_dep],c_args:['-fsanitize=address','-fsanitize-coverage=trace-cmp,trace-gep,trace-div','-g' ],
#   version : binary_age.to_string() + '.'
#             + interface_age.to_string() + '.'
#             + revision.to_string()
	     #   install : true
#)

#libvirglrenderer_dep = declare_dependency(
#   link_with: libvirglrenderer,
#   include_directories: libvirgl_inc,
#   dependencies : [libvirgl_dep, gallium_dep]
#)

install_headers('virglrenderer.h', subdir : 'virgl')


assert(cc.has_argument('-fsanitize=fuzzer'),
       'Fuzzer enabled but compiler does not support "-fsanitize=fuzzer"')
virgl_fuzzer = executable(
   'virgl_fuzzer',
   'virgl_fuzzer.c',
   virgl_sources,
   virglrenderer_sources,
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   c_args : [ '-fsanitize=fuzzer','-g' ],
   link_args : [ '-fsanitize=fuzzer','-g' ],
   dependencies : virgl_depends # [
   #  libvirglrenderer_dep,
   #  libvirgl_dep,
   #  gallium_dep, epoxy_dep]
)

virgl_fuzzer_shader = executable(
   'virgl_fuzzer_shader',
   'virgl_fuzzer_shader.c',
   virgl_sources,
   virglrenderer_sources,
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   c_args : [ '-fsanitize=fuzzer','-g' ],
   link_args : [ '-fsanitize=fuzzer','-g' ],
   dependencies : virgl_depends
)


crash_vrend_renderer_get_meminfo = executable(
   'crash_vrend_renderer_get_meminfo',
   'crash_vrend_renderer_get_meminfo.c',
   virgl_sources,
   virglrenderer_sources,
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   c_args : [ '-g' ],
   link_args : [ '-g' ],
   dependencies : virgl_depends # 
   #  libvirglrenderer_dep,
   #  libvirgl_dep,
   #  gallium_dep, epoxy_dep]
)


crash_vrend_decode_set_shader_images = executable(
   'crash_vrend_decode_set_shader_images',
   'crash_vrend_decode_set_shader_images.c',
   virgl_sources,
   virglrenderer_sources,
   vrend_sources,
   c_args : [ '-g' ],
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   dependencies : virgl_depends 
)

oom_vrend_renderer_resource_create = executable(
   'oom_vrend_renderer_resource_create',
   'oom_vrend_renderer_resource_create.c',
   virgl_sources,
   virglrenderer_sources,
   vrend_sources,
   c_args : [ '-g' ],
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   dependencies : virgl_depends
)

oom_vrend_renderer_resource_create_2 = executable(
   'oom_vrend_renderer_resource_create_2',
   'oom_vrend_renderer_resource_create_2.c',
   virgl_sources,
   virglrenderer_sources,
   vrend_sources,
   c_args : [ '-g' ],
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   dependencies : virgl_depends
)


crash_vrend_decode_clear_texture = executable(
   'crash_vrend_decode_clear_texture',
   'crash_vrend_decode_clear_texture.c',
   virgl_sources,
   virglrenderer_sources,
   vrend_sources,
   c_args : [ '-g' ],
   include_directories: [inc_gallium, inc_configuration, 'venus'],
   dependencies : virgl_depends
)


