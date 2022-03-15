#ifndef MALL_SYSTEM_PHYSICSSYSTEM_HPP_
#define MALL_SYSTEM_PHYSICSSYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include <chrono>

#include "../ComponentData/RigidBodyData.hpp"
#include "../Engine.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class PhysicsSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(PhysicsSystem, Key, Common)

    public:
        virtual void onInit()
        {
        }

        virtual void onUpdate()
        {
            auto&& lmdUpdateWorldMatrix = [&](RigidBodyData& rigidBody)
            {
                static float mem[16];
                rigidBody.rigidBody.get().getWorldTransform().getOpenGLMatrix(mem);
                rigidBody.world = glm::make_mat4(mem);
            };

            this->template forEach<RigidBodyData>(lmdUpdateWorldMatrix);
        }

        virtual void onEnd()
        {
        }

    };
}  // namespace mall

#endif