//
// Created by Khang on 8/30/2025.
//

#include "CommandPool.h"

#include <VkBootstrap.h>

bool CommandPool::init(RenderData &render_data) {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = render_data.rdVkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(render_data.rdVkbDevice.device, &poolCreateInfo, nullptr, &render_data.rdCommandPool);
    if (result != VK_SUCCESS) {
        printf("[%s:%d] error: could not create command pool (error: %i)\n", __FUNCTION__, __LINE__, result);
        return false;
    }

    return true;
}

void CommandPool::cleanup(RenderData &render_data) {
    vkDestroyCommandPool(render_data.rdVkbDevice.device, render_data.rdCommandPool, nullptr);
}