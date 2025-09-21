//
// Created by Khang on 9/14/2025.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vulkan/vulkan.h>

class Shader {
    public:
        static VkShaderModule loadShader(VkDevice device, std::string shaderFileName);
        static void cleanup(VkDevice device, VkShaderModule module);
};

#endif //SHADER_H
