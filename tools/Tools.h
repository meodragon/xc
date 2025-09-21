//
// Created by Khang on 9/14/2025.
//

#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <glm/glm.hpp>

#include <assimp/matrix4x4.h>

class Tools {
    public:
        static std::string getFilenameExt(std::string filename);
        static std::string loadFileToString(std::string fileName);

        static glm::mat4 convertAiToGLM(aiMatrix4x4 inMat);
};

#endif //TOOLS_H
