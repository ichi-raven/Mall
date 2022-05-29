#ifndef MALL_SYSTEM_PHYSICSSYSTEM_HPP_
#define MALL_SYSTEM_PHYSICSSYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

#include "../ComponentData/RigidBodyData.hpp"
#include "../Engine.hpp"

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
            {
                std::function<void(RigidBodyData&, TransformData&)>&& lmdApplyTransform = [&](RigidBodyData& rigidBody, TransformData& transform)
                {
                    if (!rigidBody.isStatic)
                        return;
                    auto& btTrans = rigidBody.rigidBody.get().getWorldTransform();
                    btTrans.setOrigin(btVector3(transform.pos.x, transform.pos.y, transform.pos.z));
                    btTrans.setRotation(btQuaternion(transform.rot.x, transform.rot.y, transform.rot.z, transform.rot.w));
                };

                this->template forEach<RigidBodyData, TransformData>(lmdApplyTransform);
            }

            {
                std::function<void(RigidBodyData&, MeshData&)>&& lmdUpdateWorldMatrix = [&](RigidBodyData& rigidBody, MeshData& mesh)
                {
                    static float mem[16];
                    if (rigidBody.isStatic)
                        return;
                    rigidBody.rigidBody.get().getWorldTransform().getOpenGLMatrix(mem);
                    mesh.world = glm::make_mat4(mem);
                };

                this->template forEach<RigidBodyData, MeshData>(lmdUpdateWorldMatrix);
            }

            {
                std::function<void(RigidBodyData&, SkeletalMeshData&)>&& lmdUpdateWorldMatrix = [&](RigidBodyData& rigidBody, MeshData& mesh)
                {
                    static float mem[16];
                    if (rigidBody.isStatic)
                        return;
                    rigidBody.rigidBody.get().getWorldTransform().getOpenGLMatrix(mem);
                    mesh.world = glm::make_mat4(mem);
                };

                this->template forEach<RigidBodyData, SkeletalMeshData>(lmdUpdateWorldMatrix);
            }
        }

        virtual void onEnd()
        {
        }
    };
}  // namespace mall

#endif