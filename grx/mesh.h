//
// Created by Khang on 8/24/2025.
//

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <glm\glm.hpp>

#include <assimp\material.h>

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

#endif //MESH_H
