#ifndef MALL_SYSTEM_TRANSFORMSYSTEM_HPP_
#define MALL_SYSTEM_TRANSFORMSYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include <chrono>

#include "../ComponentData/TransformData.hpp"
#include "../Engine.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class TransformSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(TransformSystem, Key, Common)

    public:
        virtual void onInit()
        {
        }

        virtual void onUpdate()
        {
            {
                const float& deltaTime = this->common().deltaTime;

                auto&& lmdUpdateTransform = [&](TransformData& transform)
                {
                    transform.pos += deltaTime* transform.vel += deltaTime * transform.acc;
                };

                this->template forEach<TransformData>(lmdUpdateTransform);
            }

            {
                std::function<void(TransformData&, MeshData&)>&& lmdUpdateWorldMatrix = [&](TransformData& transform, MeshData& mesh)
                {
                    mesh.world = glm::translate(glm::mat4(1.f), transform.pos) * glm::toMat4(transform.rot) * glm::scale(transform.scale);
                };

                this->template forEach<TransformData, MeshData>(lmdUpdateWorldMatrix);
            }

            {
                std::function<void(TransformData&, SkeletalMeshData&)>&& lmdUpdateWorldMatrix = [&](TransformData& transform, SkeletalMeshData& mesh)
                {
                    mesh.world = glm::translate(glm::mat4(1.f), transform.pos) * glm::toMat4(transform.rot) * glm::scale(transform.scale);
                };

                this->template forEach<TransformData, SkeletalMeshData>(lmdUpdateWorldMatrix);
            }
        }

        virtual void onEnd()
        {

        }
    };
}  // namespace mall

#endif