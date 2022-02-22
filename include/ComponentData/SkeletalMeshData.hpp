#ifndef MALL_COMPONENTDATA_SKELTALMESH_HPP_
#define MALL_COMPONENTDATA_SKELTALMESH_HPP_

#include <assimp/scene.h>

#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>

#include "MeshData.hpp"

namespace mall
{
    struct SkeletalMeshData : public MeshData
    {
        COMPONENT_DATA(SkeletalMeshData)

        struct Bone
        {
            glm::mat4 offset;
            glm::mat4 transform;
        };

        struct Skeleton
        {
            void traverseNode(float timeInAnim, size_t animationIndex, const aiNode* node, glm::mat4 parentTransform);

            std::vector<Bone> bones;
            TUPointer<const aiScene> scene;
            std::unordered_map<std::string, uint32_t> boneMap;
            glm::mat4 defaultAxis;
            glm::mat4 globalInverse;
        };

        struct RenderingInfo
        {
            struct SceneCBParam
            {
                glm::mat4 world;
                glm::mat4 view;
                glm::mat4 proj;
                float receiveShadow;
                float lighting;
                uint useBone;
                uint padding;
            };

            constexpr static std::size_t MaxBoneNum = 128;
            struct BoneCBParam
            {
                glm::mat4 boneMat[MaxBoneNum];
            };

            Cutlass::HBuffer sceneCB;
            Cutlass::HBuffer boneCB;
        };

        uint32_t animationIndex;
        TUPointer<Skeleton> skeleton;
        float timeScale;

        RenderingInfo renderingInfo;
    };
}  // namespace mall

#endif