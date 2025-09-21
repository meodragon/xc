//
// Created by Khang on 8/24/2025.
//

#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <glm\glm.hpp>

#include <assimp\material.h>

#include <surface.h>

#include <vma/vk_mem_alloc.h>

#include <VkBootstrap.h>

struct Vertex {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec2 uv = glm::vec2(0.0f);
    glm::uvec4 boneNumber = glm::uvec4(0);
    glm::vec4 boneWeight = glm::vec4(0.0f);
};

struct Mesh {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    std::unordered_map<aiTextureType, std::string> textures{};
    bool usesPBRColors = false;
};

struct VkUniformBufferData {
  size_t bufferSize = 0;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation bufferAlloc = nullptr;

  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

struct VkUploadMatrices {
  glm::mat4 viewMatrix{};
  glm::mat4 projectionMatrix{};
};

struct VkShaderStorageBufferData {
  size_t bufferSize = 0;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation bufferAlloc = nullptr;

  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

struct VkPushConstants {
	int pkModelStride;
	int pkWorldPosOffset;
};

struct RenderData {
	/* Window/Surface */
    xcSurface *surface = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

	/**/
    unsigned int rdTriangleCount = 0;
    unsigned int rdMatricesSize = 0;

    int rdFieldOfView = 60;

    float rdFrameTime = 0.0f;
    float rdMatrixGenerateTime = 0.0f;
    float rdUploadToVBOTime = 0.0f;
    float rdUploadToUBOTime = 0.0f;
    float rdUIGenerateTime = 0.0f;
    float rdUIDrawTime = 0.0f;

    /* Vulkan Objects */
	vkb::Instance rdVkbInstance{};
	vkb::PhysicalDevice rdVkbPhysicalDevice{};
  	vkb::Device rdVkbDevice{};
	VmaAllocator rdAllocator = nullptr;

	VkQueue rdGraphicsQueue = VK_NULL_HANDLE;
	VkQueue rdPresentQueue = VK_NULL_HANDLE;

	vkb::Swapchain rdVkbSwapChain{};

  	std::vector<VkImage> rdSwapChainImages{};
  	std::vector<VkImageView> rdSwapChainImageViews{};
	std::vector<VkFramebuffer> rdFramebuffers{};

  	VkFormat rdDepthFormat = VK_FORMAT_UNDEFINED;
	VkImage rdDepthImage = VK_NULL_HANDLE;
  	VmaAllocation rdDepthImageAlloc = VK_NULL_HANDLE;
  	VkImageView rdDepthImageView = VK_NULL_HANDLE;

    VkCommandPool rdCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

	VkDescriptorPool rdDescriptorPool = VK_NULL_HANDLE;

  	VkDescriptorSetLayout rdAssimpTextureDescriptorLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout rdAssimpDescriptorLayout = VK_NULL_HANDLE;
	VkDescriptorSet rdAssimpDescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSet rdAssimpSkinningDescriptorSet = VK_NULL_HANDLE;

    VkRenderPass rdRenderPass = VK_NULL_HANDLE;

	VkPipelineLayout rdAssimpPipelineLayout = VK_NULL_HANDLE;
	VkPipelineLayout rdAssimpSkinningPipelineLayout = VK_NULL_HANDLE;

	VkPipeline rdAssimpPipeline = VK_NULL_HANDLE;
	VkPipeline rdAssimpSkinningPipeline = VK_NULL_HANDLE;

	VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
	VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
	VkFence rdRenderFence = VK_NULL_HANDLE;
};

#endif //RENDER_DATA_H
