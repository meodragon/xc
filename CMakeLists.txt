cmake_minimum_required(VERSION 3.31)
project(xc)

set(CMAKE_CXX_STANDARD 23)

include_directories("$ENV{VULKAN_SDK}/Include")
set(VK_BOOTSTRAP_PATH "D:/Developer/vk-bootstrap/src")
include_directories("${VK_BOOTSTRAP_PATH}")
include_directories("$ENV{ASSIMP_SDK}/include")
include_directories(grx)
include_directories(model)
include_directories(surface)

link_directories("$ENV{VULKAN_SDK}/Lib")
link_directories("$ENV{ASSIMP_SDK}/lib/Debug")

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
        main.cpp)

target_link_libraries(xc vulkan-1.lib assimp-vc143-mtd.lib)
