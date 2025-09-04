//
// Created by Khang on 9/2/2025.
//

#include "CommandBuffer.h"

bool CommandBuffer::init(RenderData renderData, VkCommandBuffer& commandBuffer) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderData.rdCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(renderData.rdVkbDevice.device, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
        printf("[%s:%d] error: could not allocate command buffer (error: %i)\n", __FUNCTION__, __LINE__, result);
        return false;
    }
    return true;
}

void CommandBuffer::cleanup(RenderData renderData, VkCommandBuffer commandBuffer) {
    vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &commandBuffer);
}