//
// Created by Khang on 9/2/2025.
//

#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <render_data.h>

class CommandBuffer {
    public:
        static bool init(RenderData renderData, VkCommandBuffer& commandBuffer);
        static void cleanup(RenderData renderData, VkCommandBuffer commandBuffer);
};

#endif //COMMANDBUFFER_H
