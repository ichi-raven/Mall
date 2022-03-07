#include "../../include/ComponentData/TransformData.hpp"

namespace mall
{
    void TransformData::setup(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, glm::vec3 scalePerAxis, glm::quat rotationQuat, glm::quat rotationVelocity, glm::quat rotationAcceleration)
    {
        pos = position;
        vel = velocity;
        acc = acceleration;

        scale = scalePerAxis;

        rot    = rotationQuat;
        rotVel = rotationVelocity;
        rotAcc = rotationAcceleration;
    }
}  // namespace mall