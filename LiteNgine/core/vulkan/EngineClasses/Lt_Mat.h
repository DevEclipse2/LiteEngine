#pragma once

//lt math geddit?

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp> 

inline void ExtractTransformDegrees(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale)
{
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;

    if (glm::decompose(transform, outScale, orientation, outPosition, skew, perspective))
    {
        outRotation = glm::eulerAngles(orientation);

        outRotation = glm::degrees(outRotation);
    }
}