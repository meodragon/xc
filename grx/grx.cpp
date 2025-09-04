//
// Created by Khang on 8/23/2025.
//

#include "grx.h"

#include <vector>

#include <VkBootstrap.h>
#include <CommandPool.h>
#include <CommandBuffer.h>
#include <UniformBuffer.h>
#include <ShaderStorageBuffer.h>
#include <RenderPass.h>

xcGraphics::xcGraphics(xcSurface *surface) {
	xc_surface = surface;
}

bool xcGraphics::init(unsigned int width, unsigned int height) {
	surface_width = width;
	surface_height = height;

	deviceInit();
	vmaInit();
	createSwapChain();
	createDepthBuffer();
	createCommandPool();
	createCommandBuffer();
    createMatrixUBO();
	createDescriptorPool();
	createDescriptorLayouts();
	createDescriptorSets();
	createRenderPass();
	return false;
}

void xcGraphics::cleanup() {
	VkResult result = vkDeviceWaitIdle(render_data.rdVkbDevice.device);
  	if (result != VK_SUCCESS) {
    	printf("%s fatal error: could not wait for device idle (error: %i)\n", __FUNCTION__, result);
    	return;
  	}

    CommandBuffer::cleanup(render_data, render_data.rdCommandBuffer);
    CommandPool::cleanup(render_data);

    RenderPass::cleanup(render_data);

    UniformBuffer::cleanup(render_data, mPerspectiveViewMatrixUBO);
	ShaderStorageBuffer::cleanup(render_data, mBoneMatrixBuffer);
  	ShaderStorageBuffer::cleanup(render_data, mWorldPosBuffer);

    vkFreeDescriptorSets(render_data.rdVkbDevice.device, render_data.rdDescriptorPool, 1, &render_data.rdAssimpDescriptorSet);
    vkFreeDescriptorSets(render_data.rdVkbDevice.device, render_data.rdDescriptorPool, 1, &render_data.rdAssimpSkinningDescriptorSet);

    vkDestroyDescriptorSetLayout(render_data.rdVkbDevice.device, render_data.rdAssimpDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(render_data.rdVkbDevice.device, render_data.rdAssimpTextureDescriptorLayout, nullptr);

    vkDestroyDescriptorPool(render_data.rdVkbDevice.device, render_data.rdDescriptorPool, nullptr);

	vkDestroyImageView(render_data.rdVkbDevice.device, render_data.rdDepthImageView, nullptr);

  	vmaDestroyImage(render_data.rdAllocator, render_data.rdDepthImage, render_data.rdDepthImageAlloc);
  	vmaDestroyAllocator(render_data.rdAllocator);

  	render_data.rdVkbSwapChain.destroy_image_views(render_data.rdSwapChainImageViews);
  	vkb::destroy_swapchain(render_data.rdVkbSwapChain);

	vkb::destroy_device(render_data.rdVkbDevice);
  	vkb::destroy_surface(render_data.rdVkbInstance.instance, surface);
  	vkb::destroy_instance(render_data.rdVkbInstance);

	printf("[%s:%d] xc-graphics cleanup\n", __func__, __LINE__);
}

void xcGraphics::deviceInit() {
	vkb::InstanceBuilder instBuild;
	auto instRet = instBuild
    	.use_default_debug_messenger()
    	.request_validation_layers()
    	.require_api_version(1, 1, 0)
    	.build();

  	if (!instRet) {
    	printf("%s error: could not build vkb instance\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}

  	render_data.rdVkbInstance = instRet.value();

	VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = xc_surface->hWnd;
    create_info.hinstance = xc_surface->hInstance;

    if (vkCreateWin32SurfaceKHR(render_data.rdVkbInstance, &create_info, nullptr, &surface) != VK_SUCCESS)
    {
        printf("[%s:%d] failed to create surface\n", __func__, __LINE__);
		exit(EXIT_FAILURE);
    }

  	/* force anisotropy */
  	VkPhysicalDeviceFeatures requiredFeatures{};
  	requiredFeatures.samplerAnisotropy = VK_TRUE;

  	/* just get the first available device */
  	vkb::PhysicalDeviceSelector physicalDevSel{render_data.rdVkbInstance};
  	auto firstPysicalDevSelRet = physicalDevSel
    	.set_surface(surface)
    	.set_required_features(requiredFeatures)
    	.select();

  	if (!firstPysicalDevSelRet) {
    	printf("%s error: could not get physical devices\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}

  	/* a 2nd call is required to enable all the supported features, like wideLines */
  	VkPhysicalDeviceFeatures physFeatures;
  	vkGetPhysicalDeviceFeatures(firstPysicalDevSelRet.value(), &physFeatures);

  	auto secondPhysicalDevSelRet = physicalDevSel
    	.set_surface(surface)
    	.set_required_features(physFeatures)
    	.select();

  	if (!secondPhysicalDevSelRet) {
    	printf("%s error: could not get physical devices\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}

  	render_data.rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();
  	printf("%s: found physical device '%s'\n", __FUNCTION__, render_data.rdVkbPhysicalDevice.name.c_str());

  	/* required for dynamic buffer with world position matrices */
  	VkDeviceSize minSSBOOffsetAlignment = render_data.rdVkbPhysicalDevice.properties.limits.minStorageBufferOffsetAlignment;
  	printf("%s: the physical device has a minimal SSBO offset of %Ii bytes\n", __FUNCTION__, minSSBOOffsetAlignment);
  	mMinSSBOOffsetAlignment = std::max(minSSBOOffsetAlignment, sizeof(glm::mat4));
  	printf("%s: SSBO offset has been adjusted to %Ii bytes\n", __FUNCTION__, mMinSSBOOffsetAlignment);

  	vkb::DeviceBuilder devBuilder{render_data.rdVkbPhysicalDevice};
  	auto devBuilderRet = devBuilder.build();
  	if (!devBuilderRet) {
    	printf("%s error: could not get devices\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}
  	render_data.rdVkbDevice = devBuilderRet.value();
}

void xcGraphics::vmaInit() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = render_data.rdVkbPhysicalDevice.physical_device;
	allocatorInfo.device = render_data.rdVkbDevice.device;
	allocatorInfo.instance = render_data.rdVkbInstance.instance;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &render_data.rdAllocator);
	if (result != VK_SUCCESS) {
		printf("[%s:%d] error: could not init VMA (error %i)\n", __FUNCTION__, __LINE__, result);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::getQueues() {
	auto graphQueueRet = render_data.rdVkbDevice.get_queue(vkb::QueueType::graphics);
	if (!graphQueueRet.has_value()) {
		printf("[%s:%d] error: could not get graphics queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	render_data.rdGraphicsQueue = graphQueueRet.value();

	auto presentQueueRet = render_data.rdVkbDevice.get_queue(vkb::QueueType::present);
	if (!presentQueueRet.has_value()) {
		printf("[%s:%d] error: could not get present queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	render_data.rdPresentQueue = presentQueueRet.value();
}

void xcGraphics::createSwapChain() {
	vkb::SwapchainBuilder swapChainBuild{render_data.rdVkbDevice};
  	VkSurfaceFormatKHR surfaceFormat;

  	/* set surface to non-sRGB */
  	surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;

  	/* VK_PRESENT_MODE_FIFO_KHR enables vsync */
  	auto  swapChainBuildRet = swapChainBuild
    	.set_old_swapchain(render_data.rdVkbSwapChain)
    	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    	.set_desired_format(surfaceFormat)
    	.build();

  	if (!swapChainBuildRet) {
    	printf("[%s:%d] error: could not init swapchain\n", __FUNCTION__, __LINE__);
    	exit(EXIT_FAILURE);
  	}

  	vkb::destroy_swapchain(render_data.rdVkbSwapChain);
  	render_data.rdVkbSwapChain = swapChainBuildRet.value();
  	render_data.rdSwapChainImages = swapChainBuildRet.value().get_images().value();
  	render_data.rdSwapChainImageViews = swapChainBuildRet.value().get_image_views().value();
}

void xcGraphics::createDepthBuffer() {
	VkExtent3D depthImageExtent = {
        render_data.rdVkbSwapChain.extent.width,
        render_data.rdVkbSwapChain.extent.height,
        1
  	};

  	render_data.rdDepthFormat = VK_FORMAT_D32_SFLOAT;

  	VkImageCreateInfo depthImageInfo{};
  	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  	depthImageInfo.format = render_data.rdDepthFormat;
  	depthImageInfo.extent = depthImageExtent;
  	depthImageInfo.mipLevels = 1;
  	depthImageInfo.arrayLayers = 1;
  	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  	VmaAllocationCreateInfo depthAllocInfo{};
  	depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  	depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  	VkResult result = vmaCreateImage(render_data.rdAllocator, &depthImageInfo, &depthAllocInfo, &render_data.rdDepthImage, &render_data.rdDepthImageAlloc, nullptr);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not allocate depth buffer memory (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
  	}

  	VkImageViewCreateInfo depthImageViewinfo{};
  	depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  	depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  	depthImageViewinfo.image = render_data.rdDepthImage;
  	depthImageViewinfo.format = render_data.rdDepthFormat;
  	depthImageViewinfo.subresourceRange.baseMipLevel = 0;
  	depthImageViewinfo.subresourceRange.levelCount = 1;
  	depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
  	depthImageViewinfo.subresourceRange.layerCount = 1;
  	depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  	result = vkCreateImageView(render_data.rdVkbDevice.device, &depthImageViewinfo, nullptr, &render_data.rdDepthImageView);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not create depth buffer image view (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
	}
}

void xcGraphics::createCommandPool() {
	if (!CommandPool::init(render_data)) {
		printf("[%s:%d] error: could not create command pool\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createCommandBuffer() {
	if (!CommandBuffer::init(render_data, render_data.rdCommandBuffer)) {
		printf("[%s:%d] error: could not create command buffer\n]", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createMatrixUBO() {
	if (!UniformBuffer::init(render_data, mPerspectiveViewMatrixUBO)) {
		printf("[%s:%d] error: could not create matrix uniform buffers\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createSSBOs() {
	if (!ShaderStorageBuffer::init(render_data, mWorldPosBuffer)) {
    	printf("%s error: could not create world position SSBO\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}

  	if (!ShaderStorageBuffer::init(render_data, mBoneMatrixBuffer)) {
    	printf("%s error: could not create bone matrix SSBO\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}
}

void xcGraphics::createDescriptorPool() {
  	std::vector<VkDescriptorPoolSize> poolSizes =
  	{
    	{ VK_DESCRIPTOR_TYPE_SAMPLER, 10000 },
    	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 },
    	{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10000 },
    	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
  	};

  	VkDescriptorPoolCreateInfo poolInfo{};
  	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  	poolInfo.maxSets = 10000;
  	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  	poolInfo.pPoolSizes = poolSizes.data();

  	VkResult result = vkCreateDescriptorPool(render_data.rdVkbDevice.device, &poolInfo, nullptr, &render_data.rdDescriptorPool);
  	if (result != VK_SUCCESS) {
    	printf("%s error: could not init descriptor pool (error: %i)\n", __FUNCTION__, result);
    	exit(EXIT_FAILURE);
  	}
}

void xcGraphics::createDescriptorLayouts() {
  	VkResult result;

  	{
    	/* texture */
    	VkDescriptorSetLayoutBinding assimpTextureBind{};
    	assimpTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	assimpTextureBind.binding = 0;
    	assimpTextureBind.descriptorCount = 1;
    	assimpTextureBind.pImmutableSamplers = nullptr;
    	assimpTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    	std::vector<VkDescriptorSetLayoutBinding> assimpTexBindings = { assimpTextureBind };

    	VkDescriptorSetLayoutCreateInfo assimpTextureCreateInfo{};
    	assimpTextureCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    	assimpTextureCreateInfo.bindingCount = static_cast<uint32_t>(assimpTexBindings.size());
    	assimpTextureCreateInfo.pBindings = assimpTexBindings.data();

    	result = vkCreateDescriptorSetLayout(render_data.rdVkbDevice.device, &assimpTextureCreateInfo, nullptr, &render_data.rdAssimpTextureDescriptorLayout);
    	if (result != VK_SUCCESS) {
      		printf("%s error: could not create Assimp texturedescriptor set layout (error: %i)\n", __FUNCTION__, result);
      		exit(EXIT_FAILURE);
    	}
  	}

  	{
      	/* UBO/SSBO in shader */
    	VkDescriptorSetLayoutBinding assimpUboBind{};
    	assimpUboBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	assimpUboBind.binding = 0;
    	assimpUboBind.descriptorCount = 1;
    	assimpUboBind.pImmutableSamplers = nullptr;
    	assimpUboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    	VkDescriptorSetLayoutBinding assimpSsboBind{};
    	assimpSsboBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	assimpSsboBind.binding = 1;
    	assimpSsboBind.descriptorCount = 1;
    	assimpSsboBind.pImmutableSamplers = nullptr;
    	assimpSsboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    	std::vector<VkDescriptorSetLayoutBinding> assimpBindings = { assimpUboBind, assimpSsboBind };

    	VkDescriptorSetLayoutCreateInfo assimpCreateInfo{};
    	assimpCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    	assimpCreateInfo.bindingCount = static_cast<uint32_t>(assimpBindings.size());
    	assimpCreateInfo.pBindings = assimpBindings.data();

    	result = vkCreateDescriptorSetLayout(render_data.rdVkbDevice.device, &assimpCreateInfo, nullptr, &render_data.rdAssimpDescriptorLayout);
    	if (result != VK_SUCCESS) {
      		printf("%s error: could not create Assimp buffer descriptor set layout (error: %i)\n", __FUNCTION__, result);
      		exit(EXIT_FAILURE);
    	}
  	}
}

void xcGraphics::createDescriptorSets() {
  	/* non-animated models */
  	VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
  	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  	descriptorAllocateInfo.descriptorPool = render_data.rdDescriptorPool;
  	descriptorAllocateInfo.descriptorSetCount = 1;
  	descriptorAllocateInfo.pSetLayouts = &render_data.rdAssimpDescriptorLayout;

  	VkResult result = vkAllocateDescriptorSets(render_data.rdVkbDevice.device, &descriptorAllocateInfo, &render_data.rdAssimpDescriptorSet);
   	if (result != VK_SUCCESS) {
    	printf("%s error: could not allocate Assimp descriptor set (error: %i)\n", __FUNCTION__, result);
    	exit(EXIT_FAILURE);
  	}

  	/* animated models */
  	VkDescriptorSetAllocateInfo skinningDescriptorAllocateInfo{};
  	skinningDescriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  	skinningDescriptorAllocateInfo.descriptorPool = render_data.rdDescriptorPool;
  	skinningDescriptorAllocateInfo.descriptorSetCount = 1;
  	skinningDescriptorAllocateInfo.pSetLayouts = &render_data.rdAssimpDescriptorLayout;

  	result = vkAllocateDescriptorSets(render_data.rdVkbDevice.device, &skinningDescriptorAllocateInfo, &render_data.rdAssimpSkinningDescriptorSet);
  	if (result != VK_SUCCESS) {
    	printf("%s error: could not allocate Assimp Skinning descriptor set (error: %i)\n", __FUNCTION__, result);
    	exit(EXIT_FAILURE);
  	}

  	updateDescriptorSets();
}

void xcGraphics::updateDescriptorSets() {
  	printf("%s: updating descriptor sets\n", __FUNCTION__);
  	/* we must update the descriptor sets whenever the buffer size has changed */
  	{
    	/* non-animated shader */
    	VkDescriptorBufferInfo matrixInfo{};
     	matrixInfo.buffer = mPerspectiveViewMatrixUBO.buffer;
    	matrixInfo.offset = 0;
    	matrixInfo.range = VK_WHOLE_SIZE;

    	VkDescriptorBufferInfo worldPosInfo{};
    	worldPosInfo.buffer = mWorldPosBuffer.buffer;
    	worldPosInfo.offset = 0;
    	worldPosInfo.range = VK_WHOLE_SIZE;

    	VkWriteDescriptorSet matrixWriteDescriptorSet{};
    	matrixWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	matrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	matrixWriteDescriptorSet.dstSet = render_data.rdAssimpDescriptorSet;
    	matrixWriteDescriptorSet.dstBinding = 0;
    	matrixWriteDescriptorSet.descriptorCount = 1;
    	matrixWriteDescriptorSet.pBufferInfo = &matrixInfo;

    	VkWriteDescriptorSet posWriteDescriptorSet{};
    	posWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	posWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	posWriteDescriptorSet.dstSet = render_data.rdAssimpDescriptorSet;
    	posWriteDescriptorSet.dstBinding = 1;
    	posWriteDescriptorSet.descriptorCount = 1;
    	posWriteDescriptorSet.pBufferInfo = &worldPosInfo;

    	std::vector<VkWriteDescriptorSet> writeDescriptorSets = { matrixWriteDescriptorSet, posWriteDescriptorSet };

    	vkUpdateDescriptorSets(render_data.rdVkbDevice.device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
  	}

  	{
    	/* animated shader */
    	VkDescriptorBufferInfo matrixInfo{};
    	matrixInfo.buffer = mPerspectiveViewMatrixUBO.buffer;
    	matrixInfo.offset = 0;
    	matrixInfo.range = VK_WHOLE_SIZE;

    	VkDescriptorBufferInfo boneMatrixInfo{};
    	boneMatrixInfo.buffer = mBoneMatrixBuffer.buffer;
    	boneMatrixInfo.offset = 0;
    	boneMatrixInfo.range = VK_WHOLE_SIZE;

    	/* world pos matrix is identical, just needs another descriptor set */
    	VkWriteDescriptorSet matrixWriteDescriptorSet{};
    	matrixWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	matrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	matrixWriteDescriptorSet.dstSet = render_data.rdAssimpSkinningDescriptorSet;
    	matrixWriteDescriptorSet.dstBinding = 0;
    	matrixWriteDescriptorSet.descriptorCount = 1;
    	matrixWriteDescriptorSet.pBufferInfo = &matrixInfo;

    	VkWriteDescriptorSet boneMatrixWriteDescriptorSet{};
   		boneMatrixWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	boneMatrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	boneMatrixWriteDescriptorSet.dstSet = render_data.rdAssimpSkinningDescriptorSet;
    	boneMatrixWriteDescriptorSet.dstBinding = 1;
    	boneMatrixWriteDescriptorSet.descriptorCount = 1;
    	boneMatrixWriteDescriptorSet.pBufferInfo = &boneMatrixInfo;

    	std::vector<VkWriteDescriptorSet> skinningWriteDescriptorSets = { matrixWriteDescriptorSet, boneMatrixWriteDescriptorSet };

    	vkUpdateDescriptorSets(render_data.rdVkbDevice.device, static_cast<uint32_t>(skinningWriteDescriptorSets.size()), skinningWriteDescriptorSets.data(), 0, nullptr);
  	}
}

void xcGraphics::createRenderPass() {
    if (!RenderPass::init(render_data)) {
        printf("%s error: could not init renderpass\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
}