//
// Created by Khang on 9/16/2025.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <vulkan/vulkan.h>

#include "render_data.h"

class Framebuffer {
    public:
        static bool init(RenderData &renderData);
        static void cleanup(RenderData &renderData);
};

#endif //FRAMEBUFFER_H
