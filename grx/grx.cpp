//
// Created by Khang on 8/23/2025.
//

#include "grx.h"

#include <VkBootstrap.h>

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
	//createCommandPool();
	//createCommandBuffer();
	return false;
}

void xcGraphics::cleanup() {
	VkResult result = vkDeviceWaitIdle(rdVkbDevice.device);
  	if (result != VK_SUCCESS) {
    	printf("%s fatal error: could not wait for device idle (error: %i)\n", __FUNCTION__, result);
    	return;
  	}

	vkDestroyImageView(rdVkbDevice.device, rdDepthImageView, nullptr);

  	vmaDestroyImage(rdAllocator, rdDepthImage, rdDepthImageAlloc);
  	vmaDestroyAllocator(rdAllocator);

  	rdVkbSwapChain.destroy_image_views(rdSwapChainImageViews);
  	vkb::destroy_swapchain(rdVkbSwapChain);

	vkb::destroy_device(rdVkbDevice);
  	vkb::destroy_surface(rdVkbInstance.instance, surface);
  	vkb::destroy_instance(rdVkbInstance);

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

  	rdVkbInstance = instRet.value();

	VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = xc_surface->hWnd;
    create_info.hinstance = xc_surface->hInstance;

    if (vkCreateWin32SurfaceKHR(rdVkbInstance, &create_info, nullptr, &surface) != VK_SUCCESS)
    {
        printf("[%s:%d] failed to create surface\n", __func__, __LINE__);
		exit(EXIT_FAILURE);
    }

  	/* force anisotropy */
  	VkPhysicalDeviceFeatures requiredFeatures{};
  	requiredFeatures.samplerAnisotropy = VK_TRUE;

  	/* just get the first available device */
  	vkb::PhysicalDeviceSelector physicalDevSel{rdVkbInstance};
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

  	rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();
  	printf("%s: found physical device '%s'\n", __FUNCTION__, rdVkbPhysicalDevice.name.c_str());

  	/* required for dynamic buffer with world position matrices */
  	VkDeviceSize minSSBOOffsetAlignment = rdVkbPhysicalDevice.properties.limits.minStorageBufferOffsetAlignment;
  	printf("%s: the physical device has a minimal SSBO offset of %Ii bytes\n", __FUNCTION__, minSSBOOffsetAlignment);
  	mMinSSBOOffsetAlignment = std::max(minSSBOOffsetAlignment, sizeof(glm::mat4));
  	printf("%s: SSBO offset has been adjusted to %Ii bytes\n", __FUNCTION__, mMinSSBOOffsetAlignment);

  	vkb::DeviceBuilder devBuilder{rdVkbPhysicalDevice};
  	auto devBuilderRet = devBuilder.build();
  	if (!devBuilderRet) {
    	printf("%s error: could not get devices\n", __FUNCTION__);
    	exit(EXIT_FAILURE);
  	}
  	rdVkbDevice = devBuilderRet.value();
}

void xcGraphics::vmaInit() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = rdVkbPhysicalDevice.physical_device;
	allocatorInfo.device = rdVkbDevice.device;
	allocatorInfo.instance = rdVkbInstance.instance;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &rdAllocator);
	if (result != VK_SUCCESS) {
		printf("[%s:%d] error: could not init VMA (error %i)\n", __FUNCTION__, __LINE__, result);
		exit(EXIT_FAILURE);
	}
}

void xcGraphics::getQueues() {
	auto graphQueueRet = rdVkbDevice.get_queue(vkb::QueueType::graphics);
	if (!graphQueueRet.has_value()) {
		printf("[%s:%d] error: could not get graphics queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	rdGraphicsQueue = graphQueueRet.value();

	auto presentQueueRet = rdVkbDevice.get_queue(vkb::QueueType::present);
	if (!presentQueueRet.has_value()) {
		printf("[%s:%d] error: could not get present queue\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
	}
	rdPresentQueue = presentQueueRet.value();
}

void xcGraphics::createSwapChain() {
	vkb::SwapchainBuilder swapChainBuild{rdVkbDevice};
  	VkSurfaceFormatKHR surfaceFormat;

  	/* set surface to non-sRGB */
  	surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;

  	/* VK_PRESENT_MODE_FIFO_KHR enables vsync */
  	auto  swapChainBuildRet = swapChainBuild
    	.set_old_swapchain(rdVkbSwapChain)
    	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    	.set_desired_format(surfaceFormat)
    	.build();

  	if (!swapChainBuildRet) {
    	printf("[%s:%d] error: could not init swapchain\n", __FUNCTION__, __LINE__);
    	exit(EXIT_FAILURE);
  	}

  	vkb::destroy_swapchain(rdVkbSwapChain);
  	rdVkbSwapChain = swapChainBuildRet.value();
  	rdSwapChainImages = swapChainBuildRet.value().get_images().value();
  	rdSwapChainImageViews = swapChainBuildRet.value().get_image_views().value();
}

void xcGraphics::createDepthBuffer() {
	VkExtent3D depthImageExtent = {
        rdVkbSwapChain.extent.width,
        rdVkbSwapChain.extent.height,
        1
  	};

  	rdDepthFormat = VK_FORMAT_D32_SFLOAT;

  	VkImageCreateInfo depthImageInfo{};
  	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  	depthImageInfo.format = rdDepthFormat;
  	depthImageInfo.extent = depthImageExtent;
  	depthImageInfo.mipLevels = 1;
  	depthImageInfo.arrayLayers = 1;
  	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  	VmaAllocationCreateInfo depthAllocInfo{};
  	depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  	depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  	VkResult result = vmaCreateImage(rdAllocator, &depthImageInfo, &depthAllocInfo, &rdDepthImage, &rdDepthImageAlloc, nullptr);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not allocate depth buffer memory (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
  	}

  	VkImageViewCreateInfo depthImageViewinfo{};
  	depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  	depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  	depthImageViewinfo.image = rdDepthImage;
  	depthImageViewinfo.format = rdDepthFormat;
  	depthImageViewinfo.subresourceRange.baseMipLevel = 0;
  	depthImageViewinfo.subresourceRange.levelCount = 1;
  	depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
  	depthImageViewinfo.subresourceRange.layerCount = 1;
  	depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  	result = vkCreateImageView(rdVkbDevice.device, &depthImageViewinfo, nullptr, &rdDepthImageView);
  	if (result != VK_SUCCESS) {
    	printf("[%s:%d] error: could not create depth buffer image view (error: %i)\n", __FUNCTION__, __LINE__, result);
    	exit(EXIT_FAILURE);
	}
}

//void xcGraphics::createCommandPool() {}
