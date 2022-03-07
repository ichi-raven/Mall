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
            mPrev = mNow = std::chrono::high_resolution_clock::now();
            mNowSecond = 0;
        }

        virtual void onUpdate()
        {
            mNow                   = std::chrono::high_resolution_clock::now();
            const float& deltaTime = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(mNow - mPrev).count() / 1000000.);

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

                skeleton.traverseNode(fmod(mNowSecond * updateTime, scene.mAnimations[skeletalMesh.animationIndex]->mDuration), skeletalMesh.animationIndex, scene.mRootNode, glm::mat4(1.f));
            };

            this->template forEach<SkeletalMeshData>(lmdUpdateSkeleton);

            mPrev = mNow;
            mNowSecond += deltaTime;
        }

        virtual void onEnd()
        {
        }

    protected:
        std::chrono::high_resolution_clock::time_point mPrev;
        std::chrono::high_resolution_clock::time_point mNow;
        float mNowSecond;
    };
}  // namespace mall

#endif