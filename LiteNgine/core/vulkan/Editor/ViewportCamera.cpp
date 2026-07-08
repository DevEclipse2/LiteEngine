#include "ViewportCamera.h"

namespace lte{
    ViewportCamera::ViewportCamera()
    {
        updateCameraVectors();
    }

    ViewportCamera::~ViewportCamera()
    {
    }

    glm::mat4 ViewportCamera::getViewMatrix() const
    {
        return glm::lookAt(position, position + forward, up);
    }

    // Process keyboard inputs (WASD)
    void  ViewportCamera::processKeyboard(int direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == 0) position += forward * velocity; // W
        if (direction == 1) position -= forward * velocity; // S
        if (direction == 2) position -= right * velocity;   // A
        if (direction == 3) position += right * velocity;   // D
    }

    // Process mouse drag delta
    void ViewportCamera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
    {
        xOffset *= mouseSensitivity;
        yOffset *= -mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset; // Adjust sign based on your preferred inversion

        if (constrainPitch) {
            pitch = std::clamp(pitch, -89.0f, 89.0f);
        }

        updateCameraVectors();
    }
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void ViewportCamera::updateCameraVectors()
    {
        // Calculate the new forward vector
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        forward = glm::normalize(front);
        // Recalculate Right and Up vector
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }
};