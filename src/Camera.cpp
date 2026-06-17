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

Camera::Camera()
{
    Position = glm::vec3(0.0f,0.0f,3.0f);

    Front = glm::vec3(0.0f,0.0f,-1.0f);

    Up = glm::vec3(0.0f,1.0f,0.0f);

    Yaw = -90.0f;
    Pitch = 0.0f;

    Fov = 45.0f;
}

void Camera::ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime)
{
    float speed = 10.0f * deltaTime;

    if(forward)
        Position += Front * speed;

    if(backward)
        Position -= Front * speed;

    glm::vec3 cameraRight = glm::normalize(glm::cross(Front, Up));

    if(left)
        Position -= cameraRight * speed;

    if(right)
        Position += cameraRight * speed;
}

void Camera::ProcessMouse(float xoffset, float yoffset)
{
    float sensitivity = 0.05f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if(Pitch > 89.0f)
        Pitch = 89.0f;

    if(Pitch < -89.0f)
        Pitch = -89.0f;

    UpdateVectors();
}

void Camera::UpdateVectors()
{
    glm::vec3 front;

    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    Front = glm::normalize(front);

    glm::vec3 right = glm::normalize(
            glm::cross(Front,glm::vec3(0,1,0))
        );

    Up = glm::normalize(
            glm::cross(right, Front)
        );
}

void Camera::ProcessScroll(float yoffset)
{
    Fov -= yoffset;

    if(Fov < 1.0f)
        Fov = 1.0f;

    if(Fov > 45.0f)
        Fov = 45.0f;
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}