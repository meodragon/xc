//
// Created by Khang on 8/23/2025.
//

#ifndef GRX_H
#define GRX_H

//#include <surface.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

class xcGraphics {
    public:
        xcGraphics(void *surface);
		void cleanup();
	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
};

#endif //GRX_H
