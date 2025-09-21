//
// Created by Khang on 9/14/2025.
//

#ifndef SKINNINGPIPELINE_H
#define SKINNINGPIPELINE_H
#include <string>
#include <vulkan/vulkan.h>

#include "render_data.h"

class SkinningPipeline {
    public:
        static bool init(RenderData &renderData, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline, std::string vertexShaderFilename, std::string fragmentShaderFilename);
        static void cleanup(RenderData &renderData, VkPipeline &pipeline);
};

#endif //SKINNINGPIPELINE_H
