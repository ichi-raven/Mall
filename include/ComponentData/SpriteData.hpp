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

            Cutlass::HBuffer scene2DCB;
        };

        TUArray<Cutlass::HTexture> textures;
        std::uint32_t index;

        RenderingInfo renderingInfo;
    };
}  // namespace mall

#endif