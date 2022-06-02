#ifndef MALL_SYSTEM_ANIMATESYSTEM_HPP_
#define MALL_SYSTEM_ANIMATESYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include <chrono>

#include "../ComponentData/TransformData.hpp"
#include "../Engine.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class AnimateSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(AnimateSystem, Key, Common)

    public:
        virtual void onInit()
        {
            mNowSecond = 0;
        }

        virtual void onUpdate()
        {
            const double& deltaTime = this->common().deltaTime;

            auto&& lmdUpdateSkeleton = [&](SkeletalMeshData& skeletalMesh)
            {
                auto& skeleton = skeletalMesh.skeleton.get();
                auto& scene = skeleton.scene.get();
                if (skeletalMesh.animationIndex >= scene.mNumAnimations)
                {
                    assert(!"invalid animation index!");
                    return;
                }

                float updateTime = scene.mAnimations[skeletalMesh.animationIndex]->mTicksPerSecond;
                if (updateTime == 0)
                {
                    assert(!"zero update time!");
                    updateTime = 25.f;  //?
                }

                skeleton.traverseNode(fmod(mNowSecond * updateTime, scene.mAnimations[skeletalMesh.animationIndex]->mDuration), skeletalMesh.animationIndex, scene.mRootNode, glm::mat4(1.f), skeletalMesh.defaultAxis);
            };

            this->template forEach<SkeletalMeshData>(lmdUpdateSkeleton);

            mNowSecond += deltaTime;
        }

        virtual void onEnd()
        {

        }

    protected:
        float mNowSecond;
    };
}  // namespace mall

#endif