#ifndef MALL_COMPONENTDATA_MATERIALDATA_HPP_
#define MALL_COMPONENTDATA_MATERIALDATA_HPP_

#include <Cutlass/Texture.hpp>
#include <MVECS/IComponentData.hpp>
#include <string>

#include "../Utility.hpp"

namespace mall
{
    struct MaterialData : public mvecs::IComponentData
    {
        struct Texture
        {
            Cutlass::HTexture handle;
            std::string name;
            std::string path;
        };

        TUArray<Texture> textures;
    };
}  // namespace mall

#endif