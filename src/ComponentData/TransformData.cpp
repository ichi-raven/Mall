#include "../../include/Mall/ComponentData/TransformData.hpp"

namespace mall
{
    void TransformData::setup(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, glm::vec3 scalePerAxis, glm::quat rotationQuat)
    {
        pos   = position;
        vel   = velocity;
        acc   = acceleration;
        scale = scalePerAxis;
        rot   = rotationQuat;
    }
    
}  // namespace mall