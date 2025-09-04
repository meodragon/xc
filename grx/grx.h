//
// Created by Khang on 8/23/2025.
//

#ifndef GRX_H
#define GRX_H

#include <string>

#include <surface.h>

#include <glm/glm.hpp>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <render_data.h>

class xcGraphics {
    public:
        xcGraphics(xcSurface *surface);
		bool init(unsigned int width, unsigned int height);
		bool draw(float deltaTime);

		bool hasModel(std::string modelFilename);
		//std::shared_ptr<AssimpModel> getModel(std::string modelFilename);
		bool addModel(std::string modelFilename);
		bool delModel(std::string modelFilename);

		void cleanup();
	private:
		xcSurface *xc_surface = nullptr;
		unsigned int surface_width = 0;
		unsigned int surface_height = 0;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkDeviceSize mMinSSBOOffsetAlignment = 0;

		RenderData render_data;

        VkUniformBufferData mPerspectiveViewMatrixUBO{};

    	VkShaderStorageBufferData mWorldPosBuffer{};
    	VkShaderStorageBufferData mBoneMatrixBuffer{};

		void deviceInit();
		void vmaInit();
		void getQueues();
		void createSwapChain();
		void createDepthBuffer();
		void createCommandPool();
		void createCommandBuffer();
        void createMatrixUBO();
        void createSSBOs();
        void createDescriptorPool();
        void createDescriptorLayouts();
        void createDescriptorSets();
		void updateDescriptorSets();
		void createRenderPass();
};

#endif //GRX_H
