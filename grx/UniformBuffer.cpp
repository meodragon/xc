//
// Created by Khang on 9/3/2025.
//

#include <UniformBuffer.h>

bool UniformBuffer::init(RenderData& renderData, VkUniformBufferData &uboData) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(VkUploadMatrices);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VkResult result = vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocInfo, &uboData.buffer, &uboData.bufferAlloc, nullptr);
    if (result != VK_SUCCESS) {
        printf("[%s:%d] error: could not allocate uniform buffer via VMA (error: %i)\n", __FUNCTION__, __LINE__, result);
        return false;
    }

    return true;
}

void UniformBuffer::cleanup(RenderData& renderData, VkUniformBufferData &uboData) {
    vmaDestroyBuffer(renderData.rdAllocator, uboData.buffer, uboData.bufferAlloc);
}