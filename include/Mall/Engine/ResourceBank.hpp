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
#include "../ComponentData/SoundData.hpp"
#include "../ComponentData/SpriteData.hpp"
#include "../ComponentData/TextData.hpp"

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

        bool create(std::string_view path, MeshData& mesh, MaterialData& material, const glm::mat4& defaultAxis = glm::mat4(1.f));

        bool create(std::string_view path, SkeletalMeshData& skeletalMesh, MaterialData& material, const glm::mat4& defaultAxis = glm::mat4(1.f));

        bool create(const std::vector<std::string_view>& paths, std::string_view name, SpriteData& sprite);

        bool create(const std::initializer_list<std::string_view>& paths, std::string_view name, SpriteData& sprite);

        bool create(std::string_view path, std::string_view name, SpriteData& sprite);

        bool getSprite(std::string_view name, SpriteData& sprite);

        bool create(std::string_view path, SoundData& sound);
        
        bool create(std::string_view path, TextData& text);

        void destroy(MeshData& mesh, MaterialData& material);
        
        void destroy(SkeletalMeshData& mesh, MaterialData& material);
        
        void destroy(SpriteData& sprite);
        
        void destroy(TextData& text);
        
        void destroy(SoundData& sound);


        void clearCache(std::string_view pathOrName);

        void clearCacheAll();

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

        struct Sound
        {
            // PaStream* pStream;
            // float volumeRate;

            // SoundData::WaveFormat format;          //????????????????????????
            // std::vector<unsigned char> data_8bit;  //????????????(8bit?????????)
            // std::vector<short> data_16bit;         //????????????(16bit?????????)

            // int RIFFFileSize;  // RIFF?????????????????????????????????????????????
            // int PCMDataSize;   //???????????????????????????

            SoLoud::Wav wavData;
            
        };

        struct Font
        {
            stbtt_fontinfo fontInfo;
            unsigned char* fontBuffer;
        };

        void processNode(const aiNode* node, Model& model_out);

        void processMesh(const aiNode* node, const aiMesh* mesh, Model& model_out);

        void loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName, Model& model_out);

        void loadBones(const aiNode* node, const aiMesh* mesh, std::vector<VertexBoneData>& vbdata_out, SkeletalMeshData::Skeleton& skeleton_out);

        std::unordered_map<std::string, Model> mModelCacheMap;
        std::unordered_map<std::string, Model> mSkeletalModelCacheMap;
        std::unordered_map<std::string, Sprite> mSpriteCacheMap;
        std::unordered_map<std::string, Sound> mSoundCacheMap;
        std::unordered_map<std::string, Font> mFontCacheMap;

        std::unordered_map<std::string, Cutlass::HTexture> mTextureCacheMap;

        std::shared_ptr<Cutlass::Context> mpContext;

        Assimp::Importer mImporter;
    };
}  // namespace mall

#endif