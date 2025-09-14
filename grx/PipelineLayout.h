//
// Created by Khang on 9/13/2025.
//

#ifndef PIPELINELAYOUT_H
#define PIPELINELAYOUT_H

#include <render_data.h>

class PipelineLayout {
    public:
        static bool init(RenderData& renderData, VkPipelineLayout& pipelineLayout,
            std::vector<VkDescriptorSetLayout> layouts, std::vector<VkPushConstantRange> pushConstants = {});

        static void cleanup(RenderData &renderData, VkPipelineLayout &pipelineLayout);
};
#endif //PIPELINELAYOUT_H
