#ifndef MALL_COMPONENTDATA_TRANSFORMDATA_HPP_
#define MALL_COMPONENTDATA_TRANSFORMDATA_HPP_

#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mall
{
    struct TransformData : public mvecs::IComponentData
    {
        glm::vec3 mPos;
        glm::vec3 mVel;
        glm::vec3 mAcc;

        glm::vec3 mScale;

        glm::quat mRotation;
        float mRotVel;
        float mRotAcc;

        glm::mat4 mWorld;
    };
}  // namespace mall

#endif