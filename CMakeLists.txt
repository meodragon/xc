cmake_minimum_required(VERSION 3.31)
project(xc)

set(CMAKE_CXX_STANDARD 23)

include_directories("$ENV{VULKAN_SDK}/Include")
set(VK_BOOTSTRAP_PATH "D:/Developer/vk-bootstrap/src")
include_directories("${VK_BOOTSTRAP_PATH}")
include_directories("$ENV{ASSIMP_SDK}/include")
include_directories("$ENV{IMGUI_SDK}")
include_directories("$ENV{IMGUI_SDK}/backends")
include_directories(grx)
include_directories(model)
include_directories(surface)
include_directories(tools)

link_directories("$ENV{VULKAN_SDK}/Lib")
link_directories("$ENV{ASSIMP_SDK}/lib/Debug")

set(SHADER_FILE_LIST "assimp.vert" "assimp.frag" "assimp_skinning.vert" "assimp_skinning.frag")
foreach(item IN LISTS SHADER_FILE_LIST)
    add_custom_command(
            OUTPUT ${item}.spv
            COMMAND $ENV{VULKAN_SDK}/Bin/glslc.exe
                -c ${CMAKE_SOURCE_DIR}/shader/${item}
                -o ${CMAKE_BINARY_DIR}/${item}.spv
            DEPENDS ${CMAKE_SOURCE_DIR}/shader/${item}
            COMMENT "Compile shader: ${item}"
            VERBATIM
    )
    list(APPEND COMPILED_SHADER_FILES ${CMAKE_BINARY_DIR}/${item}.spv)
endforeach()

string(REPLACE "\\" "/" IMGUI_SRC $ENV{IMGUI_SDK})
file(GLOB IMGUI "${IMGUI_SRC}/imgui*.cpp" "${IMGUI_SRC}/backends/imgui_impl_vulkan.cpp"
        "${IMGUI_SRC}/backends/imgui_impl_win32.cpp")

add_executable(xc
        grx/CommandBuffer.cpp
        grx/CommandBuffer.h
        grx/CommandPool.cpp
        grx/CommandPool.h
        grx/grx.cpp
        grx/grx.h
        grx/render_data.h
        grx/RenderPass.cpp
        grx/RenderPass.h
        grx/ShaderStorageBuffer.cpp
        grx/ShaderStorageBuffer.h
        grx/UniformBuffer.cpp
        grx/UniformBuffer.h
        model/model.cpp
        model/model.h
        model/model_and_instance_data.h
        surface/icon.rc
        surface/resource.h
        surface/surface.cpp
        surface/surface.h
        "${VK_BOOTSTRAP_PATH}/VkBootstrap.cpp"
        main.cpp
        grx/PipelineLayout.cpp
        grx/PipelineLayout.h
        ${COMPILED_SHADER_FILES}
        grx/SkinningPipeline.cpp
        grx/SkinningPipeline.h
        grx/Shader.cpp
        grx/Shader.h
        tools/Tools.cpp
        tools/Tools.h
        grx/Framebuffer.cpp
        grx/Framebuffer.h
        grx/SyncObjects.cpp
        grx/SyncObjects.h
        grx/UserInterface.cpp
        grx/UserInterface.h
        ${IMGUI})

target_link_libraries(xc vulkan-1.lib assimp-vc143-mtd.lib)
