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
	createSSBOs();
	createDescriptorPool();
	createDescriptorLayouts();
	createDescriptorSets();
	createRenderPass();
	return false;
}

void xcGraphics::cleanup() {
	VkResult result = vkDeviceWaitIdle(renderData.rdVkbDevice.device);
  	if (result != VK_SUCCESS) {
    	printf("%s fatal error: could not wait for device idle (error: %i)\n", __FUNCTION__, result);
    	return;
  	}

    CommandBuffer::cleanup(renderData, renderData.rdCommandBuffer);
    CommandPool::cleanup(renderData);

    RenderPass::cleanup(renderData);

    UniformBuffer::cleanup(renderData, mPerspectiveViewMatrixUBO);
	ShaderStorageBuffer::cleanup(renderData, mBoneMatrixBuffer);
  	ShaderStorageBuffer::cleanup(renderData, mWorldPosBuffer);

    vkFreeDescriptorSets(renderData.rdVkbDevice.device, renderData.rdDescriptorPool, 1,
    	&renderData.rdAssimpDescriptorSet);
    vkFreeDescriptorSets(renderData.rdVkbDevice.device, renderData.rdDescriptorPool, 1,
    	&renderData.rdAssimpSkinningDescriptorSet);

    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdAssimpDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdAssimpTextureDescriptorLayout, nullptr);

    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdDescriptorPool, nullptr);

	vkDestroyImageView(renderData.rdVkbDevice.device, renderData.rdDepthImageView, nullptr);

  	vmaDestroyImage(renderData.rdAllocator, renderData.rdDepthImage, renderData.rdDepthImageAlloc);
  	vmaDestroyAllocator(renderData.rdAllocator);

  	renderData.rdVkbSwapChain.destroy_image_views(renderData.rdSwapChainImageViews);
  	vkb::destroy_swapchain(renderData.rdVkbSwapChain);

	vkb::destroy_device(renderData.rdVkbDevice);
  	vkb::destroy_surface(renderData.rdVkbInstance.instance, surface);
  	vkb::destroy_instance(renderData.rdVkbInstance);

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

  	renderData.rdVkbInstance = instRet.value();

	VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = xc_surface->hWnd;
    create_info.hinstance = xc_surface->hInstance;

    if (vkCreateWin32SurfaceKHR(renderData.rdVkbInstance, &create_info, nullptr, &surface) != VK_SUCCESS)
    {
        printf("[%s:%d] failed to create surface\n", __func__, __LINE__);
		exit(EXIT_FAILURE);
    }

  	/* force anisotropy */
  	VkPhysicalDeviceFeatures requiredFeatures{};
  	requiredFeatures.samplerAnisotropy = VK_TRUE;

  	/* just get the first available device */
  	vkb::PhysicalDeviceSelector physicalDevSel{renderData.rdVkbInstance};
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

  	renderData.rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();
  	printf("%s: found physical device '%s'\n", __FUNCTION__, renderData.rdVkbPhysicalDevice.name.c_str());

  	/* required for dynamic buffer with world position matrices */
  	VkDeviceSize minSSBOOffsetAlignment = renderData.rdVkbPhysicalDevice.properties.limits.minStorageBufferOffsetAlignment;
  	printf("%s: the physical device has a minimal SSBO offset of %Ii bytes\n", __FUNCTION__, minSSBOOffsetAlignment);
  	mMinSSBOOffsetAlignment = std::max(minSSBOOffsetAlignment, sizeof(glm::mat4));
  	printf("%s: SSBO offset has been adjusted to %Ii bytes\n", __FUNCTION__, mMinSSBOOffsetAlignment);

  	vkb::DeviceBuilder devBuilder{renderData.rdVkbPhysicalDevice};
  	auto devBuilderRet = devBuilder.build();
  	if (!devBuilderRet) {
    	printf("%s error: could not get devices\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}
  	renderData.rdVkbDevice = devBuilderRet.value();
}

void xcGraphics::vmaInit() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = renderData.rdVkbPhysicalDevice.physical_device;
	allocatorInfo.device = renderData.rdVkbDevice.device;
	allocatorInfo.instance = renderData.rdVkbInstance.instance;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &renderData.rdAllocator);
	if (result != VK_SUCCESS) {
		printf("[%s:%d] error: could not init VMA (error %i)\n", __FUNCTION__, __LINE__, result);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::getQueues() {
	auto graphQueueRet = renderData.rdVkbDevice.get_queue(vkb::QueueType::graphics);
	if (!graphQueueRet.has_value()) {
		printf("[%s:%d] error: could not get graphics queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	renderData.rdGraphicsQueue = graphQueueRet.value();

	auto presentQueueRet = renderData.rdVkbDevice.get_queue(vkb::QueueType::present);
	if (!presentQueueRet.has_value()) {
		printf("[%s:%d] error: could not get present queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	renderData.rdPresentQueue = presentQueueRet.value();
}

void xcGraphics::createSwapChain() {
	vkb::SwapchainBuilder swapChainBuild{renderData.rdVkbDevice};
  	VkSurfaceFormatKHR surfaceFormat;

  	/* set surface to non-sRGB */
  	surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;

  	/* VK_PRESENT_MODE_FIFO_KHR enables vsync */
  	auto  swapChainBuildRet = swapChainBuild
    	.set_old_swapchain(renderData.rdVkbSwapChain)
    	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    	.set_desired_format(surfaceFormat)
    	.build();

  	if (!swapChainBuildRet) {
    	printf("[%s:%d] error: could not init swap chain\n", __FUNCTION__, __LINE__);
    	exit(EXIT_FAILURE);
  	}

  	vkb::destroy_swapchain(renderData.rdVkbSwapChain);
  	renderData.rdVkbSwapChain = swapChainBuildRet.value();
  	renderData.rdSwapChainImages = swapChainBuildRet.value().get_images().value();
  	renderData.rdSwapChainImageViews = swapChainBuildRet.value().get_image_views().value();
}

void xcGraphics::createDepthBuffer() {
	VkExtent3D depthImageExtent = {
        renderData.rdVkbSwapChain.extent.width,
        renderData.rdVkbSwapChain.extent.height,
        1
  	};

  	renderData.rdDepthFormat = VK_FORMAT_D32_SFLOAT;

  	VkImageCreateInfo depthImageInfo{};
  	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  	depthImageInfo.format = renderData.rdDepthFormat;
  	depthImageInfo.extent = depthImageExtent;
  	depthImageInfo.mipLevels = 1;
  	depthImageInfo.arrayLayers = 1;
  	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  	VmaAllocationCreateInfo depthAllocInfo{};
  	depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  	depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  	VkResult result = vmaCreateImage(renderData.rdAllocator, &depthImageInfo, &depthAllocInfo, &renderData.rdDepthImage,
  		&renderData.rdDepthImageAlloc, nullptr);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not allocate depth buffer memory (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
  	}

  	VkImageViewCreateInfo depthImageViewinfo{};
  	depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  	depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  	depthImageViewinfo.image = renderData.rdDepthImage;
  	depthImageViewinfo.format = renderData.rdDepthFormat;
  	depthImageViewinfo.subresourceRange.baseMipLevel = 0;
  	depthImageViewinfo.subresourceRange.levelCount = 1;
  	depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
  	depthImageViewinfo.subresourceRange.layerCount = 1;
  	depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  	result = vkCreateImageView(renderData.rdVkbDevice.device, &depthImageViewinfo, nullptr, &renderData.rdDepthImageView);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not create depth buffer image view (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
	}
}

void xcGraphics::createCommandPool() {
	if (!CommandPool::init(renderData)) {
		printf("[%s:%d] error: could not create command pool\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createCommandBuffer() {
	if (!CommandBuffer::init(renderData, renderData.rdCommandBuffer)) {
		printf("[%s:%d] error: could not create command buffer\n]", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createMatrixUBO() {
	if (!UniformBuffer::init(renderData, mPerspectiveViewMatrixUBO)) {
		printf("[%s:%d] error: could not create matrix uniform buffers\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::createSSBOs() {
	if (!ShaderStorageBuffer::init(renderData, mWorldPosBuffer)) {
    	printf("%s error: could not create world position SSBO\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}

  	if (!ShaderStorageBuffer::init(renderData, mBoneMatrixBuffer)) {
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

  	VkResult result = vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
  		&renderData.rdDescriptorPool);
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

    	result = vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &assimpTextureCreateInfo,
    		nullptr, &renderData.rdAssimpTextureDescriptorLayout);
    	if (result != VK_SUCCESS) {
      		printf("%s error: could not create Assimp texture descriptor set layout (error: %i)\n", __FUNCTION__, result);
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

    	result = vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &assimpCreateInfo,
    		nullptr, &renderData.rdAssimpDescriptorLayout);
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
  	descriptorAllocateInfo.descriptorPool = renderData.rdDescriptorPool;
  	descriptorAllocateInfo.descriptorSetCount = 1;
  	descriptorAllocateInfo.pSetLayouts = &renderData.rdAssimpDescriptorLayout;

  	VkResult result = vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo, &renderData.rdAssimpDescriptorSet);
   	if (result != VK_SUCCESS) {
    	printf("%s error: could not allocate Assimp descriptor set (error: %i)\n", __FUNCTION__, result);
    	exit(EXIT_FAILURE);
  	}

  	/* animated models */
  	VkDescriptorSetAllocateInfo skinningDescriptorAllocateInfo{};
  	skinningDescriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  	skinningDescriptorAllocateInfo.descriptorPool = renderData.rdDescriptorPool;
  	skinningDescriptorAllocateInfo.descriptorSetCount = 1;
  	skinningDescriptorAllocateInfo.pSetLayouts = &renderData.rdAssimpDescriptorLayout;

  	result = vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &skinningDescriptorAllocateInfo,
  		&renderData.rdAssimpSkinningDescriptorSet);
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
    	matrixWriteDescriptorSet.dstSet = renderData.rdAssimpDescriptorSet;
    	matrixWriteDescriptorSet.dstBinding = 0;
    	matrixWriteDescriptorSet.descriptorCount = 1;
    	matrixWriteDescriptorSet.pBufferInfo = &matrixInfo;

    	VkWriteDescriptorSet posWriteDescriptorSet{};
    	posWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	posWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	posWriteDescriptorSet.dstSet = renderData.rdAssimpDescriptorSet;
    	posWriteDescriptorSet.dstBinding = 1;
    	posWriteDescriptorSet.descriptorCount = 1;
    	posWriteDescriptorSet.pBufferInfo = &worldPosInfo;

    	const std::vector writeDescriptorSets = { matrixWriteDescriptorSet, posWriteDescriptorSet };

    	vkUpdateDescriptorSets(renderData.rdVkbDevice.device, static_cast<uint32_t>(writeDescriptorSets.size()),
    		writeDescriptorSets.data(), 0, nullptr);
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
    	matrixWriteDescriptorSet.dstSet = renderData.rdAssimpSkinningDescriptorSet;
    	matrixWriteDescriptorSet.dstBinding = 0;
    	matrixWriteDescriptorSet.descriptorCount = 1;
    	matrixWriteDescriptorSet.pBufferInfo = &matrixInfo;

    	VkWriteDescriptorSet boneMatrixWriteDescriptorSet{};
   		boneMatrixWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	boneMatrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	boneMatrixWriteDescriptorSet.dstSet = renderData.rdAssimpSkinningDescriptorSet;
    	boneMatrixWriteDescriptorSet.dstBinding = 1;
    	boneMatrixWriteDescriptorSet.descriptorCount = 1;
    	boneMatrixWriteDescriptorSet.pBufferInfo = &boneMatrixInfo;

    	std::vector<VkWriteDescriptorSet> skinningWriteDescriptorSets = { matrixWriteDescriptorSet, boneMatrixWriteDescriptorSet };

    	vkUpdateDescriptorSets(renderData.rdVkbDevice.device, static_cast<uint32_t>(skinningWriteDescriptorSets.size()),
    		skinningWriteDescriptorSets.data(), 0, nullptr);
  	}
}

void xcGraphics::createRenderPass() {
    if (!RenderPass::init(renderData)) {
        printf("%s error: could not init render pass\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
}