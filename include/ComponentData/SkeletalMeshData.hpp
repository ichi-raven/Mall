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

        uint32_t animationIndex;
        TUPointer<Skeleton> skeleton;
        float timeScale;
    };
}  // namespace mall

#endif