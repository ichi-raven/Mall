#ifndef MALL_SYSTEM_RENDERSYSTEM_HPP_
#define MALL_SYSTEM_RENDERSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
#include "../Engine/Graphics.hpp"

namespace mall
{
    template <typename Key, typename Common>
    class RenderSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(RenderSystem, Key, Common)

    public:
        virtual void onInit()
        {
        }

        virtual void onUpdate()
        {
            forEach<MeshData, MaterialData>([&](MeshData& mesh, MaterialData& material)
                                            { std::unique_ptr<Graphics>& graphics = common()->graphics; });
        }

        virtual void onEnd()
        {
        }
    };
}  // namespace mall

#endif