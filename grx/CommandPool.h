//
// Created by Khang on 8/30/2025.
//

#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H
#include <vulkan/vulkan.h>
#include <render_data.h>

class CommandPool {
    public:
        static bool init(RenderData &render_data);
        static void cleanup(RenderData &render_data);
};
#endif //COMMANDPOOL_H
