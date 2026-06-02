#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

    float Yaw;
    float Pitch;
    float Fov;

    Camera();

    glm::mat4 GetViewMatrix();

    void ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime);

    void ProcessMouse(float xoffset, float yoffset);

    void ProcessScroll(float yoffset);

private:
    void UpdateVectors();
};