//
// Created by Khang on 9/20/2025.
//

#ifndef SYNCOBJECTS_H
#define SYNCOBJECTS_H

#include <vulkan/vulkan.h>

#include "render_data.h"

class SyncObjects {
    public:
        static bool init(RenderData &renderData);
        static void cleanup(RenderData &renderData);
};

#endif //SYNCOBJECTS_H
