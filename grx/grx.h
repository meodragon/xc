//
// Created by Khang on 8/23/2025.
//

#ifndef GRX_H
#define GRX_H

#include <string>

#include <surface.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

class xcGraphics {
    public:
        xcGraphics(xcSurface *surface);

		bool draw(float deltaTime);

		bool hasModel(std::string modelFilename);
		//std::shared_ptr<AssimpModel> getModel(std::string modelFilename);
		bool addModel(std::string modelFilename);
		bool delModel(std::string modelFilename);

		void cleanup();
	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
};

#endif //GRX_H
