//
// Created by Khang on 9/16/2025.
//

#include "Framebuffer.h"

bool Framebuffer::init(RenderData &renderData) {

    renderData.rdFramebuffers.resize(renderData.rdSwapChainImageViews.size());

    for (unsigned int i = 0; i < renderData.rdSwapChainImageViews.size(); ++i) {
        std::vector<VkImageView> attachments = { renderData.rdSwapChainImageViews.at(i), renderData.rdDepthImageView };

        VkFramebufferCreateInfo FboInfo{};
        FboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FboInfo.renderPass = renderData.rdRenderPass;
        FboInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        FboInfo.pAttachments = attachments.data();
        FboInfo.width = renderData.rdVkbSwapChain.extent.width;
        FboInfo.height = renderData.rdVkbSwapChain.extent.height;
        FboInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(renderData.rdVkbDevice.device, &FboInfo, nullptr, &renderData.rdFramebuffers.at(i));
        if (result != VK_SUCCESS) {
            printf("%s error: failed to create framebuffer %i (error: %i)\n", __FUNCTION__, i, result);
            return false;
        }
    }
    return true;
}

void Framebuffer::cleanup(RenderData &renderData) {
    for (auto &fb : renderData.rdFramebuffers) {
        vkDestroyFramebuffer(renderData.rdVkbDevice.device, fb, nullptr);
    }
}