#ifndef MALL_SYSTEM_ENGINEBASICSYSTEM_HPP_
#define MALL_SYSTEM_ENGINEBASICSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../Engine.hpp"
#include "AnimateSystem.hpp"
#include "AudioSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "TextSystem.hpp"
#include "TransformSystem.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class EngineBasicSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(EngineBasicSystem, Key, Common)

    public:
        enum class DefaultExecOrder
        {
            eAnimateSystem   = std::numeric_limits<int>::max() - (1 << 6),
            eAudioSystem     = std::numeric_limits<int>::max() - (1 << 6),
            eTextSystem      = std::numeric_limits<int>::max() - (1 << 6),
            eTransformSystem = std::numeric_limits<int>::max() - (1 << 5),
            ePhysicsSystem   = std::numeric_limits<int>::max() - (1 << 4),
            eRenderSystem    = std::numeric_limits<int>::max() - (1 << 3),
        };

        virtual void onInit()
        {
            this->template addSystem<mall::AnimateSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::eAnimateSystem));
            this->template addSystem<mall::AudioSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::eAudioSystem));
            this->template addSystem<mall::TextSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::eTextSystem));
            this->template addSystem<mall::TransformSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::eTransformSystem));
            this->template addSystem<mall::PhysicsSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::ePhysicsSystem));
            this->template addSystem<mall::RenderSystem<Key, Common>>(static_cast<int>(DefaultExecOrder::eRenderSystem));
            // this->mRemoveThisSystem = true;
        }

        virtual void onUpdate()
        {
            if (this->common().input->getKey(Cutlass::Key::Escape))
                this->endAll();
        }

        virtual void onEnd()
        {
        }
    };

}  // namespace mall

#endif