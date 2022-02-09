#include "../../include/Engine/ResourceBank.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <regex>

namespace mall
{
    inline glm::mat4 convert4x4(const aiMatrix4x4& from)
    {
        glm::mat4 to;

        // transpose
        for (uint8_t i = 0; i < 4; ++i)
            for (uint8_t j = 0; j < 4; ++j)
                to[i][j] = from[j][i];

        return to;
    }

    inline glm::quat convertQuat(const aiQuaternion& from)
    {
        glm::quat to;
        to.w = from.w;
        to.x = from.x;
        to.y = from.y;
        to.z = from.z;
        return to;
    }

    inline glm::vec3 convertVec3(const aiVector3D& from)
    {
        glm::vec3 to;
        to.x = from.x;
        to.y = from.y;
        to.z = from.z;
        return to;
    }

    inline glm::vec2 convertVec2(const aiVector2D& from)
    {
        glm::vec2 to;
        to.x = from.x;
        to.y = from.y;
        return to;
    }

    ResourceBank::ResourceBank(const std::shared_ptr<Cutlass::Context>& context)
        : mContext(context)
    {
    }

    ResourceBank::~ResourceBank()
    {
        mModelCacheMap.clear();
        mSkeletalModelCacheMap.clear();
        mTextureCacheMap.clear();

        mContext.reset();
    }

    bool ResourceBank::load(std::string_view path, MeshData& meshData, MaterialData& materialData)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mModelCacheMap.find(strPath);
        if (iter == mModelCacheMap.end())
        {
            auto& model = iter->second;
            model.path  = strPath;

            model.pScene = mImporter.ReadFile(model.path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!model.pScene.value() || model.pScene.value()->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model.pScene.value()->mRootNode)
            {
                std::cerr << "ERROR::ASSIMP::" << mImporter.GetErrorString() << "\n";
                assert(!"failed to import model!");
                return false;
            }

            processNode(model.pScene.value()->mRootNode, model);
            iter = mModelCacheMap.emplace(model.path, model).first;
        }
        std::cerr << "all data has been loaded!\n";

        {  // write to component data
            auto& model = iter->second;

            meshData.meshes.create(model.meshes.data(), model.meshes.size());
            meshData.loaded = true;

            for (auto& mesh : meshData.meshes)
            {
                Cutlass::BufferInfo bi;
                bi.setVertexBuffer<MeshData::Vertex>(mesh.vertices.size());
                mContext->createBuffer(bi, mesh.VB);
                bi.setIndexBuffer<std::uint32_t>(mesh.indices.size());
                mContext->createBuffer(bi, mesh.IB);
            }

            materialData.textures.create(model.material.textures.data(), model.material.textures.size());
        }

        return true;
    }

    bool ResourceBank::load(std::string_view path, SkeletalMeshData& skeletalMeshData, MaterialData& materialData)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mSkeletalModelCacheMap.find(strPath);
        if (iter == mSkeletalModelCacheMap.end())
        {
            auto& model = iter->second;
            model.path  = strPath;

            model.pScene = mImporter.ReadFile(model.path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!model.pScene.value() || model.pScene.value()->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model.pScene.value()->mRootNode)
            {
                std::cerr << "ERROR::ASSIMP::" << mImporter.GetErrorString() << "\n";
                assert(!"failed to import model!");
                return false;
            }

            model.skeleton = SkeletalMeshData::Skeleton();
            processNode(model.pScene.value()->mRootNode, model);
            iter = mSkeletalModelCacheMap.emplace(model.path, model).first;
        }
        std::cerr << "all data has been loaded!\n";

        {  // write to component data
            auto& model = iter->second;

            skeletalMeshData.meshes.create(model.meshes.data(), model.meshes.size());
            skeletalMeshData.loaded = true;
            skeletalMeshData.skeleton.create(&model.skeleton.value());
            skeletalMeshData.skeleton.get().scene.create(model.pScene.value());
            skeletalMeshData.animationIndex = 0;
            skeletalMeshData.timeScale      = 0;

            for (auto& mesh : skeletalMeshData.meshes)
            {
                Cutlass::BufferInfo bi;
                bi.setVertexBuffer<MeshData::Vertex>(mesh.vertices.size());
                mContext->createBuffer(bi, mesh.VB);
                bi.setIndexBuffer<std::uint32_t>(mesh.indices.size());
                mContext->createBuffer(bi, mesh.IB);
            }

            materialData.textures.create(model.material.textures.data(), model.material.textures.size());
        }

        return true;
    }

    bool ResourceBank::load(const std::vector<std::string_view>& paths, std::string_view name, SpriteData& spriteData)
    {
        auto&& strName = std::string(name);
        auto&& iter    = mSpriteCacheMap.find(strName);
        if (iter == mSpriteCacheMap.end())
        {
            iter = mSpriteCacheMap.emplace(strName, Sprite()).first;
            iter->second.textures.reserve(paths.size());

            for (auto& path : paths)
            {
                auto&& texIter = mTextureCacheMap.find(std::string(path));
                if (texIter == mTextureCacheMap.end())
                {
                    Cutlass::HTexture texture;
                    auto res = mContext->createTextureFromFile(path.data(), texture);
                    if (res != Cutlass::Result::eSuccess)
                    {
                        std::cerr << "failed to load texture!\npath : " << path << "\n";
                        assert(!"failed to load texture!");
                        return false;
                    }

                    texIter = mTextureCacheMap.emplace(path, texture).first;
                }

                iter->second.textures.emplace_back(texIter->second);
            }
        }

        spriteData.textures.create(iter->second.textures.data(), iter->second.textures.size());
        spriteData.index = 0;

        return true;
    }

    void ResourceBank::processNode(const aiNode* node, Model& model_out)
    {
        auto& scene = model_out.pScene.value();

        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            if (mesh)
                processMesh(node, mesh, model_out);
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], model_out);
        }
    }

    void ResourceBank::processMesh(const aiNode* node, const aiMesh* mesh, Model& model_out)
    {
        auto& scene = model_out.pScene.value();

        std::cerr << "start process mesh\n";
        // Data to fill
        auto& targetMesh = model_out.meshes.emplace_back();
        auto& vertices   = targetMesh.vertices;
        auto& indices    = targetMesh.indices;

        // Walk through each of the mesh's vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshData::Vertex vertex;

            vertex.pos = convertVec3(mesh->mVertices[i]);

            if (mesh->mNormals)
                vertex.normal = convertVec3(mesh->mNormals[i]);
            else
                vertex.normal = glm::vec3(0);

            if (mesh->mTextureCoords[0])
            {
                vertex.uv.x = (float)mesh->mTextureCoords[0][i].x;
                vertex.uv.y = (float)mesh->mTextureCoords[0][i].y;
            }
            else
            {
                assert(!"texture coord nothing!");
                vertex.uv.x = 0;
                vertex.uv.y = 0;
            }

            vertices.push_back(vertex);
        }

        std::vector<VertexBoneData> vbdata;
        if (model_out.skeleton)
        {
            loadBones(node, mesh, vbdata, model_out.skeleton.value());
            //頂点にボーン情報を付加
            for (uint32_t i = 0; i < vertices.size(); ++i)
            {
                vertices[i].joint  = glm::make_vec4(vbdata[i].id);
                vertices[i].weight = glm::make_vec4(vbdata[i].weights);
                // std::cerr << to_string(vertices[i].weight0) << "\n";
                //  float sum = vertices[i].weight.x + vertices[i].weight.y + vertices[i].weight.z + vertices[i].weight.w;
                //  assert(sum <= 1.01f);
                //  assert(vertices[i].joint.x < mBoneNum);
                //  assert(vertices[i].joint.y < mBoneNum);
                //  assert(vertices[i].joint.z < mBoneNum);
                //  assert(vertices[i].joint.w < mBoneNum);
            }
        }

        if (mesh->mFaces)
        {
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
        }

        std::cerr << "indices\n";

        if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Cutlass::HTexture> textures;
            loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", model_out);
        }

        std::cerr << "materials\n";

        std::cerr << "end process mesh\n";
    }

    void ResourceBank::loadBones(const aiNode* node, const aiMesh* mesh, std::vector<VertexBoneData>& vbdata_out, SkeletalMeshData::Skeleton& skeleton_out)
    {
        vbdata_out.resize(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumBones; i++)
        {
            uint32_t boneIndex = 0;
            std::string boneName(mesh->mBones[i]->mName.data);

            if (skeleton_out.boneMap.count(boneName) <= 0)  //そのボーンは登録されてない
            {
                boneIndex = skeleton_out.bones.size();
                skeleton_out.bones.emplace_back();
            }
            else  //あった
                boneIndex = skeleton_out.boneMap[boneName];

            skeleton_out.boneMap[boneName]       = boneIndex;
            skeleton_out.bones[boneIndex].offset = convert4x4(mesh->mBones[i]->mOffsetMatrix);
            // for(size_t i = 0; i < 3; ++i)
            //     if(abs(mSkeleton.bones[boneIndex].offset[i][3]) > std::numeric_limits<float>::epsilon())
            //     {
            //         std::cerr << boneIndex << "\n";
            //         std::cerr << glm::to_string(mSkeleton.bones[boneIndex].offset) << "\n";
            //         assert(!"invalid bone offset!");
            //     }
            skeleton_out.bones[boneIndex].transform = glm::mat4(1.f);
            //頂点セット
            for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
            {
                size_t vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight    = mesh->mBones[i]->mWeights[j].mWeight;
                for (uint32_t k = 0; k < 4; k++)
                {
                    if (vbdata_out[vertexID].weights[k] == 0.0)
                    {
                        vbdata_out[vertexID].id[k]      = boneIndex;
                        vbdata_out[vertexID].weights[k] = weight;
                        break;
                    }

                    assert(k == 3 || !"invalid bone weight!");
                }
            }
        }
    }

    void ResourceBank::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName, Model& model_out)
    {
        auto& scene = model_out.pScene.value();

        std::vector<MaterialData::Texture> textures;
        std::uint32_t textureNum = mat->GetTextureCount(type);
        textures.reserve(textureNum);

        for (std::uint32_t i = 0; i < textureNum; i++)
        {
            aiString path;
            mat->GetTexture(type, i, &path);
            auto&& iter = mTextureCacheMap.find(path.C_Str());
            if (iter != mTextureCacheMap.end())
            {
                textures.emplace_back(MaterialData::Texture{iter->second, std::string(typeName), std::string(path.C_Str())});
                break;
            }

            MaterialData::Texture& texture = textures.emplace_back();

            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
            if (embeddedTexture != nullptr)
            {
                Cutlass::TextureInfo ti;
                ti.setSRTex2D(embeddedTexture->mWidth, embeddedTexture->mHeight, true);
                Cutlass::Result res = mContext->createTexture(ti, texture.handle);
                if (res != Cutlass::Result::eSuccess)
                    assert(!"failed to create embedded texture!");
                res = mContext->writeTexture(embeddedTexture->pcData, texture.handle);
                if (res != Cutlass::Result::eSuccess)
                    assert(!"failed to write to embedded texture!");
            }
            else
            {
                std::string filename = std::regex_replace(path.C_Str(), std::regex("\\\\"), "/");
                filename             = model_out.path.substr(0, model_out.path.find_last_of("/\\")) + '/' + filename;
                // std::wstring filenamews = std::wstring(filename.begin(), filename.end());
                Cutlass::Result res = mContext->createTextureFromFile(filename.c_str(), texture.handle);
                if (res != Cutlass::Result::eSuccess)
                {
                    std::cerr << "failed to load texture!\npath : " << filename << "\n";
                    assert(!"failed to create material texture!");
                }
            }
            texture.name = typeName;
            texture.path = std::string(path.C_Str());
        }

        model_out.material.textures.insert(model_out.material.textures.end(), textures.begin(), textures.end());
    }
}  // namespace mall

