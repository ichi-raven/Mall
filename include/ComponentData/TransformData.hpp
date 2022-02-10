#ifndef MALL_COMPONENTDATA_TRANSFORMDATA_HPP_
#define MALL_COMPONENTDATA_TRANSFORMDATA_HPP_

#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mall
{
    struct TransformData : public mvecs::IComponentData
    {
        glm::vec3 pos;
        glm::vec3 vel;
        glm::vec3 acc;

        glm::vec3 scale;

        glm::quat rotation;
        float rotVel;
        float rotAcc;

        // glm::mat4 world;
    };
}  // namespace mall

#endif