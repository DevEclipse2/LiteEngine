#pragma once
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace lte {
	class ViewportCamera
	{
    public:
        glm::vec3 position{ 0.0f, 0.0f, 3.0f };
        glm::vec3 forward{ 0.0f, 0.0f, -1.0f };
        glm::vec3 up{ 0.0f, 1.0f, 0.0f };
        glm::vec3 right{ 1.0f, 0.0f, 0.0f };
        const glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

        // Euler Angles
        float yaw{ -90.0f }; // Start looking down the -Z axis
        float pitch{ 0.0f };

        // Camera options
        float movementSpeed{ 2.5f };
        float mouseSensitivity{ 0.1f };
        void processKeyboard(int direction, float deltaTime);
        void processMouseMovement(float xOffset, float yOffset, bool constrainPitch);
        glm::mat4 getViewMatrix() const;
        ViewportCamera();
        ~ViewportCamera();

    private:
        void updateCameraVectors();
	};
}
