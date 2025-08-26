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
	//initVma();
	//getQueues();
	//createSwapChain();
	return false;
}

void xcGraphics::cleanup() {
	VkResult result = vkDeviceWaitIdle(rdVkbDevice.device);
  	if (result != VK_SUCCESS) {
    	printf("%s fatal error: could not wait for device idle (error: %i)\n", __FUNCTION__, result);
    	return;
  	}

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