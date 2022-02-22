#ifndef MALL_RESOURCE_BANK_HPP_
#define MALL_RESOURCE_BANK_HPP_

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <Cutlass/Context.hpp>
#include <assimp/Importer.hpp>
#include <memory>

#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
#include "../ComponentData/SkeletalMeshData.hpp"
#include "../ComponentData/SpriteData.hpp"

namespace mall
{
    struct SkeletalMeshData;
    struct MaterialData;
    struct SpriteData;

    class ResourceBank
    {
    public:
        ResourceBank(const std::shared_ptr<Cutlass::Context>& context);

        ~ResourceBank();

        bool load(std::string_view path, MeshData& mesh, MaterialData& material);

        bool load(std::string_view path, SkeletalMeshData& skeletalMesh, MaterialData& material);

        bool load(const std::vector<std::string_view>& paths, std::string_view name, SpriteData& sprite);

        bool unload(std::string_view name);

    private:
        struct VertexBoneData
        {
            VertexBoneData()
            {
                weights[0] = weights[1] = weights[2] = weights[3] = 0;
                id[0] = id[1] = id[2] = id[3] = 0;
            }

            float weights[4];
            float id[4];
        };

        struct Sprite
        {
            std::vector<Cutlass::HTexture> textures;
        };

        struct Model
        {
            struct Material
            {
                std::vector<MaterialData::Texture> textures;
            };

            std::string path;
            std::vector<MeshData::Mesh> meshes;
            Material material;
            std::optional<SkeletalMeshData::Skeleton> skeleton;
            std::optional<const aiScene*> pScene;
        };

        void processNode(const aiNode* node, Model& model_out);

        void processMesh(const aiNode* node, const aiMesh* mesh, Model& model_out);

        void loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName, Model& model_out);

        void loadBones(const aiNode* node, const aiMesh* mesh, std::vector<VertexBoneData>& vbdata_out, SkeletalMeshData::Skeleton& skeleton_out);

        std::unordered_map<std::string, Model> mModelCacheMap;
        std::unordered_map<std::string, Model> mSkeletalModelCacheMap;
        std::unordered_map<std::string, Sprite> mSpriteCacheMap;

        std::unordered_map<std::string, Cutlass::HTexture> mTextureCacheMap;

        std::shared_ptr<Cutlass::Context> mpContext;

        Assimp::Importer mImporter;
    };
}  // namespace mall

#endif