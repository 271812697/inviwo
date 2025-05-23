# Inviwo OpenGL Module
ivw_module(OpenGL)

set(HEADER_FILES
    include/modules/opengl/buffer/buffergl.h
    include/modules/opengl/buffer/bufferglconverter.h
    include/modules/opengl/buffer/bufferglobjectid.h
    include/modules/opengl/buffer/bufferobject.h
    include/modules/opengl/buffer/bufferobjectarray.h
    include/modules/opengl/buffer/bufferobjectobserver.h
    include/modules/opengl/buffer/framebufferobject.h
    include/modules/opengl/buffer/renderbufferobject.h
    include/modules/opengl/canvasgl.h
    include/modules/opengl/canvasprocessorgl.h
    include/modules/opengl/clockgl.h
    include/modules/opengl/debugmessages.h
    include/modules/opengl/geometry/meshgl.h
    include/modules/opengl/glformats.h
    include/modules/opengl/image/imagecompositor.h
    include/modules/opengl/image/imagegl.h
    include/modules/opengl/image/layergl.h
    include/modules/opengl/image/layerglconverter.h
    include/modules/opengl/inviwoopengl.h
    include/modules/opengl/openglcapabilities.h
    include/modules/opengl/openglexception.h
    include/modules/opengl/openglmodule.h
    include/modules/opengl/openglmoduledefine.h
    include/modules/opengl/openglsettings.h
    include/modules/opengl/openglutils.h
    include/modules/opengl/rendering/meshdrawergl.h
    include/modules/opengl/rendering/texturequadrenderer.h
    include/modules/opengl/shader/fileshaderresource.h
    include/modules/opengl/shader/linenumberresolver.h
    include/modules/opengl/shader/shader.h
    include/modules/opengl/shader/shadermanager.h
    include/modules/opengl/shader/shaderobject.h
    include/modules/opengl/shader/shaderresource.h
    include/modules/opengl/shader/shadersegment.h
    include/modules/opengl/shader/shadertype.h
    include/modules/opengl/shader/shaderutils.h
    include/modules/opengl/shader/standardshaders.h
    include/modules/opengl/shader/stringshaderresource.h
    include/modules/opengl/sharedopenglresources.h
    include/modules/opengl/texture/buffertexture.h
    include/modules/opengl/texture/texture.h
    include/modules/opengl/texture/texture2d.h
    include/modules/opengl/texture/texture2darray.h
    include/modules/opengl/texture/texture3d.h
    include/modules/opengl/texture/textureobserver.h
    include/modules/opengl/texture/textureunit.h
    include/modules/opengl/texture/textureutils.h
    include/modules/opengl/volume/volumegl.h
    include/modules/opengl/volume/volumeglconverter.h
    include/modules/opengl/volume/volumeutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/buffer/buffergl.cpp
    src/buffer/bufferglconverter.cpp
    src/buffer/bufferobject.cpp
    src/buffer/bufferobjectarray.cpp
    src/buffer/framebufferobject.cpp
    src/buffer/renderbufferobject.cpp
    src/canvasgl.cpp
    src/canvasprocessorgl.cpp
    src/clockgl.cpp
    src/debugmessages.cpp
    src/geometry/meshgl.cpp
    src/glformats.cpp
    src/image/imagecompositor.cpp
    src/image/imagegl.cpp
    src/image/layergl.cpp
    src/image/layerglconverter.cpp
    src/inviwoopengl.cpp
    src/openglcapabilities.cpp
    src/openglexception.cpp
    src/openglmodule.cpp
    src/openglsettings.cpp
    src/openglutils.cpp
    src/rendering/meshdrawergl.cpp
    src/rendering/texturequadrenderer.cpp
    src/shader/fileshaderresource.cpp
    src/shader/linenumberresolver.cpp
    src/shader/shader.cpp
    src/shader/shadermanager.cpp
    src/shader/shaderobject.cpp
    src/shader/shaderresource.cpp
    src/shader/shadersegment.cpp
    src/shader/shadertype.cpp
    src/shader/shaderutils.cpp
    src/shader/standardshaders.cpp
    src/shader/stringshaderresource.cpp
    src/sharedopenglresources.cpp
    src/texture/buffertexture.cpp
    src/texture/texture.cpp
    src/texture/texture2d.cpp
    src/texture/texture2darray.cpp
    src/texture/texture3d.cpp
    src/texture/textureunit.cpp
    src/texture/textureutils.cpp
    src/volume/volumegl.cpp
    src/volume/volumeglconverter.cpp
    src/volume/volumeutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/composite.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometryrendering.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometryrendering.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_color.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_copy.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_identity.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_noise.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_texturequad.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_texturequad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/isoraycasting.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/layerrendering.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/layerrendering.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/minimal.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/picking.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/raycasting.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/rendertexturequad.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/rendertexturequad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/standard.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/standard.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/texturedgeometryrendering.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/texturedgeometryrendering.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/blend.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/classification.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/colorconversion.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/compositing.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/depth.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/glyphs.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/gradients.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/intersection.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/isosurface.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/pickingutils.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/raycastgeometry.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/sampler2d.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/sampler3d.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/selectioncolor.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/shading.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/structs.glsl
)
ivw_group("Shader Files" ${SHADER_FILES})

# Unit tests
set(TEST_FILES
    tests/unittests/opengl-unittest-main.cpp
    tests/unittests/shaderobject-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(
    PACKAGES GLEW OpenGL
    FILES ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES}
)

find_package(GLEW REQUIRED)
ivw_vcpkg_install(glew MODULE OpenGL)
target_link_libraries(inviwo-module-opengl PUBLIC GLEW::GLEW)

find_package(OpenGL REQUIRED)
target_link_libraries(inviwo-module-opengl PUBLIC ${OPENGL_LIBRARIES})
target_include_directories(inviwo-module-opengl PUBLIC ${OPENGL_INCLUDE_DIR})

# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
