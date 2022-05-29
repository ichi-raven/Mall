#include "../../include/Mall/ComponentData/LightData.hpp"

namespace mall
{
    void LightData::setupPointLight(glm::vec4 color_, float range_)
    {
        enable = true;
        type = LightType::ePoint;

        color = color_;
        range = range_;
    }

    void LightData::setupDirectionalLight(glm::vec4 color_, glm::vec3 direction_)
    {
        enable = true;
        type = LightType::eDirectional;

        color = color_;
        direction = direction_;
    }
}