#pragma once

#include <string>
#include <glm/glm.hpp>

int loadSimpleOBJ(std::string filePATH, int& nVertices);
struct Material
{
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    float Ns;
};

extern Material material;