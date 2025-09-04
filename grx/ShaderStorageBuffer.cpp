//
// Created by Khang on 9/4/2025.
//

#include "ShaderStorageBuffer.h"

#include <VkBootstrap.h>

bool ShaderStorageBuffer::init(RenderData& renderData, VkShaderStorageBufferData &SSBOData, size_t bufferSize) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VkResult result = vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocInfo,
      &SSBOData.buffer, &SSBOData.bufferAlloc, nullptr);
    if (result != VK_SUCCESS) {
        printf("%s error: could not allocate SSBO via VMA (error: %i)\n", __FUNCTION__, result);
        return false;
    }

    SSBOData.bufferSize = bufferSize;
    printf("%s: created SSBO of size %zi\n", __FUNCTION__, bufferSize);
    return true;
}

void ShaderStorageBuffer::cleanup(RenderData& renderData, VkShaderStorageBufferData &SSBOData) {
    vmaDestroyBuffer(renderData.rdAllocator, SSBOData.buffer, SSBOData.bufferAlloc);
}
