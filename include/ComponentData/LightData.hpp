#ifndef MALL_COMPONENTDATA_LIGHTDATA_HPP_
#define MALL_COMPONENTDATA_LIGHTDATA_HPP_

#include <Cutlass/Texture.hpp>
#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>

namespace mall
{
    struct LightData : public mvecs::IComponentData
    {
        COMPONENT_DATA(LightData)

        struct RenderingInfo
        {
            constexpr static std::size_t MaxLightNum = 16;

            struct LightCBParam
            {
                // for packing
                glm::vec3 lightDir;
                uint32_t lightType;
                glm::vec4 lightColor;
                glm::vec3 lightPos;
                float lightRange;
            };

            struct ShadowCBParam
            {
                glm::mat4 lightViewProj;
                glm::mat4 lightViewProjBias;
            };
        };

        enum class LightType
        {
            eDirectional = 1,
            ePoint = 2,
        };

        void setupPointLight(glm::vec4 color, float range);
        void setupDirectionalLight(glm::vec4 color, glm::vec3 direction);

        bool enable;
        LightType type;

        glm::vec4 color;
        glm::vec3 direction;
        float range;

        Cutlass::HTexture shadowMap;
    };
}  // namespace mall

#endif