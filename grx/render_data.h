//
// Created by Khang on 8/24/2025.
//

#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <glm\glm.hpp>

#include <assimp\material.h>

#include <surface.h>

struct Vertex {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec2 uv = glm::vec2(0.0f);
    glm::uvec4 boneNumber = glm::uvec4(0);
    glm::vec4 boneWeight = glm::vec4(0.0f);
};

struct Mesh {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    std::unordered_map<aiTextureType, std::string> textures{};
    bool usesPBRColors = false;
};

struct RenderData {
	/* Window/Surface */
    xcSurface *surface = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

	/**/
    unsigned int rdTriangleCount = 0;
    unsigned int rdMatricesSize = 0;

    int rdFieldOfView = 60;

    float rdFrameTime = 0.0f;
    float rdMatrixGenerateTime = 0.0f;
    float rdUploadToVBOTime = 0.0f;
    float rdUploadToUBOTime = 0.0f;
    float rdUIGenerateTime = 0.0f;
    float rdUIDrawTime = 0.0f;

    /* Vulkan Objects */
};

#endif //RENDER_DATA_H
