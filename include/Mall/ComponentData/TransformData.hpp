#ifndef MALL_COMPONENTDATA_TRANSFORMDATA_HPP_
#define MALL_COMPONENTDATA_TRANSFORMDATA_HPP_

#include <MVECS/IComponentData.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace mall
{
    struct TransformData : public mvecs::IComponentData
    {
        COMPONENT_DATA(TransformData)

        void setup(glm::vec3 pos = glm::vec3(0, 0, 0), glm::vec3 vel = glm::vec3(0, 0, 0), glm::vec3 acc = glm::vec3(0, 0, 0), glm::vec3 scalePerAxis = glm::vec3(1.f), glm::quat rotation = glm::quat(glm::vec3(0, 0, 0)));

        glm::vec3 pos;
        glm::vec3 vel;
        glm::vec3 acc;

        glm::vec3 scale;

        glm::quat rot;
    };
}  // namespace mall

#endif