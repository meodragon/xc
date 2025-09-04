//
// Created by Khang on 9/4/2025.
//

#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <render_data.h>

class RenderPass {
    public:
        static bool init(RenderData &renderPass);
        static void cleanup(RenderData &renderPass);
};

#endif //RENDERPASS_H
