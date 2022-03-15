#ifndef MALL_SYSTEM_ENGINEBASICSYSTEM_HPP_
#define MALL_SYSTEM_ENGINEBASICSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../Engine.hpp"
#include "AnimateSystem.hpp"
#include "AudioSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "SampleSystem.hpp"
#include "TextSystem.hpp"
#include "TransformSystem.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class EngineBasicSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(EngineBasicSystem, Key, Common)

    public:
        virtual void onInit()
        {
            this->template addSystem<mall::RenderSystem<WorldKey, Common>>();
            this->template addSystem<mall::PhysicsSystem<WorldKey, Common>>();
            this->template addSystem<mall::TransformSystem<WorldKey, Common>>();
            this->template addSystem<mall::AnimateSystem<WorldKey, Common>>();
            this->template addSystem<mall::AudioSystem<WorldKey, Common>>();
            this->template addSystem<mall::TextSystem<WorldKey, Common>>();
            this->mRemoveThisSystem = true;
        }

        virtual void onUpdate()
        {
        }

        virtual void onEnd()
        {
        }
    };

}  // namespace mall

#endif