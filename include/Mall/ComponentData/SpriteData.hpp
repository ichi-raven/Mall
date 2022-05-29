#ifndef MALL_COMPONENTDATA_SPRITEDATA_HPP_
#define MALL_COMPONENTDATA_SPRITEDATA_HPP_

#include <Cutlass/Texture.hpp>
#include <MVECS/IComponentData.hpp>

#include "../Utility.hpp"

namespace mall
{
    struct SpriteData : public mvecs::IComponentData
    {
        COMPONENT_DATA(SpriteData)

        struct RenderingInfo
        {
            struct Scene2DCBParam
            {
                glm::mat4 proj;
            };

            struct Vertex
            {
                glm::vec3 pos;
                glm::vec2 uv;
            };

            Cutlass::HBuffer spriteVB;
        };

        TUArray<Cutlass::HTexture> textures;
        std::uint32_t index;
        bool centerFlag;

        RenderingInfo renderingInfo;
    };
}  // namespace mall

#endif