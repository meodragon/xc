//
// Created by Khang on 9/3/2025.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <vulkan/vulkan.h>
#include <render_data.h>

class UniformBuffer {
    public:
        static bool init(RenderData &renderData, VkUniformBufferData &uboData);
        static void cleanup(RenderData &renderData, VkUniformBufferData &uboData);
};

#endif //UNIFORMBUFFER_H
