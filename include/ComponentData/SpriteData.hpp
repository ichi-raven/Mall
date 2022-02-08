#ifndef MALL_COMPONENTDATA_SPRITEDATA_HPP_
#define MALL_COMPONENTDATA_SPRITEDATA_HPP_

#include <Cutlass/Texture.hpp>
#include <MVECS/IComponentData.hpp>

#include "../Utility.hpp"

namespace mall
{
    struct SpriteData : public mvecs::IComponentData
    {
        TUArray<Cutlass::HTexture> textures;
        std::uint32_t index;
    };
}  // namespace mall

#endif