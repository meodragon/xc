//
// Created by Khang on 9/4/2025.
//

#ifndef SHADERSTORAGEBUFFER_H
#define SHADERSTORAGEBUFFER_H

#include <render_data.h>

class ShaderStorageBuffer {
    public:
        static bool init(RenderData &renderData, VkShaderStorageBufferData &SSBOData, size_t bufferSize = 1024);
        static void cleanup(RenderData &renderData, VkShaderStorageBufferData &SSBOData);
};

#endif //SHADERSTORAGEBUFFER_H
