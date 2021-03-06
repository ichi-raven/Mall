#include "../../include/Mall/Engine/ResourceBank.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <regex>

#define STBI_WRITE_NO_STDIO
#define STBI_MSC_SECURE_CRT

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
        : mpContext(context)
    {
    }

    ResourceBank::~ResourceBank()
    {
        clearCacheAll();
        mpContext.reset();
        std::cerr << "Resource Bank shut down\n";
    }

    bool ResourceBank::create(std::string_view path, MeshData& meshData, MaterialData& materialData, const glm::mat4& defaultAxis)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mModelCacheMap.find(strPath);

        if (iter == mModelCacheMap.end())
        {
            iter        = mModelCacheMap.emplace(strPath, Model()).first;
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

            for (auto& mesh : model.meshes)
            {
                Cutlass::BufferInfo bi;
                bi.setVertexBuffer<MeshData::Vertex>(mesh.vertices.size());
                mpContext->createBuffer(bi, mesh.VB);
                mpContext->writeBuffer(mesh.vertices.size() * sizeof(MeshData::Vertex), mesh.vertices.data(), mesh.VB);
                bi.setIndexBuffer<std::uint32_t>(mesh.indices.size());
                mpContext->createBuffer(bi, mesh.IB);
                mpContext->writeBuffer(mesh.indices.size() * sizeof(std::uint32_t), mesh.indices.data(), mesh.IB);
            }

            std::cerr << "new data loaded!\n";
        }

        {  // write to component data
            auto& model = iter->second;

            meshData.meshes.create(model.meshes.data(), model.meshes.size());
            meshData.loaded      = true;
            meshData.defaultAxis = defaultAxis;

            materialData.textures.create(model.material.textures.data(), model.material.textures.size());
        }

        {
            Cutlass::BufferInfo bi;
            bi.setUniformBuffer<MeshData::RenderingInfo::SceneCBParam>();
            mpContext->createBuffer(bi, meshData.renderingInfo.sceneCB);
        }

        return true;
    }

    void ResourceBank::destroy(MeshData& meshData, MaterialData& materialData)
    {
        mpContext->destroyBuffer(meshData.renderingInfo.sceneCB);
    }

    bool ResourceBank::create(std::string_view path, SkeletalMeshData& skeletalMeshData, MaterialData& materialData, const glm::mat4& defaultAxis)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mSkeletalModelCacheMap.find(strPath);
        if (iter == mSkeletalModelCacheMap.end())
        {
            iter        = mSkeletalModelCacheMap.emplace(strPath, Model()).first;
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

            for (auto& mesh : model.meshes)
            {
                Cutlass::BufferInfo bi;
                bi.setVertexBuffer<MeshData::Vertex>(mesh.vertices.size());
                mpContext->createBuffer(bi, mesh.VB);
                mpContext->writeBuffer(mesh.vertices.size() * sizeof(MeshData::Vertex), mesh.vertices.data(), mesh.VB);
                bi.setIndexBuffer<std::uint32_t>(mesh.indices.size());
                mpContext->createBuffer(bi, mesh.IB);
                mpContext->writeBuffer(mesh.indices.size() * sizeof(std::uint32_t), mesh.indices.data(), mesh.IB);
                std::cerr << "mesh vertices : " << mesh.vertices.size() << "\n";
                std::cerr << "mesh indices : " << mesh.indices.size() << "\n";

            }

            std::cerr << "new data loaded!\n";
        }

        {  // write to component data
            auto& model = iter->second;

            skeletalMeshData.meshes.create(model.meshes.data(), model.meshes.size());
            skeletalMeshData.loaded      = true;
            skeletalMeshData.defaultAxis = defaultAxis;
            skeletalMeshData.skeleton.create(&model.skeleton.value());
            skeletalMeshData.skeleton.get().scene.create(model.pScene.value());
            skeletalMeshData.skeleton.get().globalInverse = glm::mat4(1.f);

            skeletalMeshData.animationIndex = 0;
            skeletalMeshData.timeScale      = 1.f;

           

            if (!model.material.textures.empty())
            {
                materialData.textures.create(model.material.textures.data(), model.material.textures.size());
            }
        }

        {
            Cutlass::BufferInfo bi;
            bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::SceneCBParam>();
            mpContext->createBuffer(bi, skeletalMeshData.renderingInfo.sceneCB);

            bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::BoneCBParam>();
            mpContext->createBuffer(bi, skeletalMeshData.renderingInfo.boneCB);
        }

        return true;
    }

    void ResourceBank::destroy(SkeletalMeshData& skeletalMeshData, MaterialData& material)
    {
        mpContext->destroyBuffer(skeletalMeshData.renderingInfo.sceneCB);
        mpContext->destroyBuffer(skeletalMeshData.renderingInfo.boneCB);
    }

    bool ResourceBank::create(const std::initializer_list<std::string_view>& paths, std::string_view name, SpriteData& sprite)
    {
        return create(std::vector<std::string_view>(paths.begin(), paths.end()), name, sprite);
    }

    bool ResourceBank::create(std::string_view path, std::string_view name, SpriteData& sprite)
    {
        return create(std::vector<std::string_view>(1, path), name, sprite);
    }

    bool ResourceBank::create(const std::vector<std::string_view>& paths, std::string_view name, SpriteData& spriteData)
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
                    auto res = mpContext->createTextureFromFile(path.data(), texture);
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

        {
            Cutlass::BufferInfo bi;
            bi.setVertexBuffer<SpriteData::RenderingInfo::Vertex>(4);
            mpContext->createBuffer(bi, spriteData.renderingInfo.spriteVB);
        }

        return true;
    }

    bool ResourceBank::getSprite(std::string_view name, SpriteData& spriteData)
    {
        auto&& strName = std::string(name);
        auto&& iter    = mSpriteCacheMap.find(strName);
        if (iter == mSpriteCacheMap.end())
            return false;

        spriteData.textures.create(iter->second.textures.data(), iter->second.textures.size());
        spriteData.index = 0;

        {
            Cutlass::BufferInfo bi;
            bi.setVertexBuffer<SpriteData::RenderingInfo::Vertex>(4);
            mpContext->createBuffer(bi, spriteData.renderingInfo.spriteVB);
        }

        return true;
    }

    void ResourceBank::destroy(SpriteData& spriteData)
    {
        mpContext->destroyBuffer(spriteData.renderingInfo.spriteVB);
    }

    bool ResourceBank::create(std::string_view path, SoundData& soundData)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mSoundCacheMap.find(strPath);
        if (iter == mSoundCacheMap.end())
        {
            iter         = mSoundCacheMap.emplace(strPath, Sound()).first;
            Sound& sound = iter->second;

            auto res = sound.wavData.load(path.data());
            if (0 != res)
            {
                std::cerr << "result : " << res << "\n";
                assert(!"failed to load sound data!");
                return false;
            }

            // FILE* fp;
            // fp = fopen(path.data(), "rb");

            // if (!fp)
            // {
            //     assert(!"failed to open file!");
            //     return false;
            // }

            // std::string readbuf;  //??????
            // readbuf.resize(4);
            // int readbuf2;  //?????????????????????

            // // RIFF?????????
            // fread((char*)readbuf.c_str(), 4, 1, fp);  // 4byte?????????"RIFF"????????????
            // if (readbuf != "RIFF")
            // {
            //     assert(!"failed to load!");
            //     return false;
            // }

            // fread(&sound.RIFFFileSize, 4, 1, fp);  // 4byte????????????????????????????????????????????? (????????????????????? - 8)?????????
            // // WAVE????????????
            // fread((char*)readbuf.c_str(), 4, 1, fp);  // 4byte?????????"WAVE"????????????
            // if (readbuf != "WAVE")
            // {
            //     assert(!"failed to load!");
            //     return false;
            // }

            // //????????????????????????
            // fread((char*)readbuf.c_str(), 4, 1, fp);  // 4byte?????????"fmt "????????????
            // if (readbuf != "fmt ")
            // {
            //     assert(!"failed to load!");
            //     return false;
            // }

            // // mFormat???????????????????????????
            // fread(&readbuf2, 4, 1, fp);  // 4byte??????????????????PCM?????????16?????????

            // //?????????1
            // fread(&sound.format.format_id, 2, 1, fp);        // 2byte??????????????????PCM?????????1?????????
            // fread(&sound.format.channel, 2, 1, fp);          // 2byte??????????????????????????????1???????????????????????????2?????????
            // fread(&sound.format.sampling_rate, 4, 1, fp);    // 4byte?????????44.1kHz?????????44100?????????
            // fread(&sound.format.bytes_per_sec, 4, 1, fp);    // 4byte?????????44.1kHz 16bit ?????????????????????44100??2??2 = 176400
            // fread(&sound.format.block_size, 2, 1, fp);       // 2byte?????????16bit ?????????????????????2??2 = 4
            // fread(&sound.format.bits_per_sample, 2, 1, fp);  // 2byte?????????16bit?????????16

            // //???????????????????????????????????????????????????
            // if (sound.format.format_id != 1)
            // {
            //     assert(!"failed to load!");
            //     return false;
            // }

            // // data????????????
            // fread((char*)readbuf.c_str(), 4, 1, fp);  // 4byte?????????"data"????????????
            // if (readbuf != "data")
            // {
            //     std::cout << readbuf << "\n";
            //     assert(!"failed to load audio file!");
            //     return false;
            // }

            // fread(&sound.PCMDataSize, 4, 1, fp);  // 4byte???????????????????????????????????????????????????

            // //?????????????????????????????????????????????????????????????????????????????????????????????
            // if (sound.format.bits_per_sample == 8)
            // {
            //     // 8?????????
            //     sound.data_8bit.resize(sound.PCMDataSize + 1);                                               // reserve????????????????????????
            //     fread(&sound.data_8bit[0], 1, (size_t)2 * sound.PCMDataSize / sound.format.block_size, fp);  // vector???1byte?????????????????????
            // }
            // else if (sound.format.bits_per_sample == 16)
            // {
            //     // 16?????????
            //     sound.data_16bit.resize(sound.PCMDataSize / 2 + 2);                                           // reserve????????????????????????
            //     fread(&sound.data_16bit[0], 2, (size_t)2 * sound.PCMDataSize / sound.format.block_size, fp);  // vector???2byte?????????????????????
            // }

            // fclose(fp);
            // // pastream??????---------------------------------------
            // int (*callback)(const void* inputBuffer, void* outputBuffer,
            //                 unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
            //                 PaStreamCallbackFlags statusFlags, void* userData) =
            //     [](
            //         const void* inputBuffer,
            //         void* outputBuffer,
            //         unsigned long framesPerBuffer,
            //         const PaStreamCallbackTimeInfo* timeInfo,
            //         PaStreamCallbackFlags statusFlags,
            //         void* userData) -> int
            // {
            //     SoundData* data = (SoundData*)userData;  // userData???WAVE*??????????????????????????????
            //     float* out      = (float*)outputBuffer;  //?????????output???????????????
            //     (void)timeInfo;
            //     (void)statusFlags;  //?????????????????????????????????????????????????????????????????????)
            //     //???????????????????????????
            //     for (int i = 0; i < (int)framesPerBuffer; i++)
            //     {
            //         //??????????????????????????????L,R,L,R,...????????????????????????WAV?????????????????????
            //         // output???????????????????????????????????????
            //         for (int c = 0; c < (int)data->format.channel; ++c)
            //         {
            //             //?????????????????????????????????
            //             //????????????????????????????????????????????????????????????????????????????????????
            //             bool end = false;
            //             if (!data->loaded)
            //                 end = true;

            //             if (data->format.bits_per_sample == 8)
            //             {
            //                 end = data->bufPos >= (int)data->data_8bit.size();
            //             }
            //             else if (data->format.bits_per_sample == 16)
            //             {
            //                 end = data->bufPos >= (int)data->data_16bit.size();
            //             }

            //             if (end && !data->loopFlag)
            //             {
            //                 return (int)paComplete;
            //             }
            //             *out++ = data->volumeRate * (data->readData<float>()) / 32767.0f;  //-1.0???1.0????????????(????????????short????????????????????????,float?????????)
            //         }
            //     }

            //     return paContinue;
            // };

            // PaStreamParameters outputParameters;
            // {
            //     //???????????????????????????????????????(?????????????????????????????????????????????????????????????????????)
            //     outputParameters.device                    = Pa_GetDefaultOutputDevice();                                         //??????????????????????????????
            //     outputParameters.channelCount              = sound.format.channel;                                                //?????????????????????????????????
            //     outputParameters.sampleFormat              = paFloat32;                                                           // paFloat32(PortAudio????????????)???
            //     outputParameters.suggestedLatency          = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;  //????????????????????????
            //     outputParameters.hostApiSpecificStreamInfo = NULL;                                                                //??????????????????
            // }

            // PaStream* stream;
            // auto err = Pa_OpenStream(
            //     &stream,                       //??????????????????????????????????????????????????????????????????
            //     NULL,                          //?????????????????????????????????????????????NULL
            //     &outputParameters,             //????????????????????????
            //     44100,                         // mFormat.sampling_rate,  //44100Hz
            //     paFramesPerBufferUnspecified,  // 1???????????????????????????????????????(??????)
            //     paNoFlag,                      //?????????????????????????????????????????????????????????0
            //     callback,                      //?????????????????????????????????????????????
            //     &soundData);                   // wav?????????????????????????????????????????????????????????????????????????????????

            // sound.pStream = stream;

            // if (err != paNoError)
            // {  //???????????????
            //     Pa_Terminate();
            //     std::cerr << "error : " << err << "\n";
            //     std::cerr << sound.format.format_id << "\n";
            //     std::cerr << sound.format.channel << "\n";
            //     std::cerr << sound.format.sampling_rate << "\n";
            //     std::cerr << sound.format.bytes_per_sec << "\n";
            //     std::cerr << sound.format.block_size << "\n";
            //     std::cerr << sound.format.bits_per_sample << "\n";
            //     assert(!"failed to open stream!");
            //     return false;
            // }
        }

        soundData.loaded          = true;
        soundData.playFlag        = false;
        soundData.playingDuration = 0;
        soundData.volumeRate      = 1.f;
        soundData.runningHandle   = -1;
        soundData.pWavData.create(&iter->second.wavData);

        // soundData.ppStream.create(&iter->second.pStream);
        // soundData.volumeRate = 1.f;
        // soundData.format     = iter->second.format;

        // if (!iter->second.data_8bit.empty())
        //     soundData.data_8bit.create(iter->second.data_8bit.data(), iter->second.data_8bit.size());
        // else
        //     soundData.data_16bit.create(iter->second.data_16bit.data(), iter->second.data_16bit.size());

        // soundData.RIFFFileSize    = iter->second.RIFFFileSize;
        // soundData.PCMDataSize     = iter->second.PCMDataSize;
        // soundData.bufPos          = 0;
        // soundData.loopFlag        = true;

        return true;
    }

    void ResourceBank::destroy(SoundData& soundData)
    {
        // do nothing for now
    }

    bool ResourceBank::create(std::string_view path, TextData& text)
    {
        auto&& strPath = std::string(path);
        auto&& iter    = mFontCacheMap.find(strPath);

        if (iter == mFontCacheMap.end())
        {
            iter       = mFontCacheMap.emplace(strPath, Font()).first;
            auto& font = iter->second;

            /* Load font (. ttf) file */
            long int size = 0;
            // unsigned char *fontBuffer = NULL;

            FILE* fontFile = NULL;
            fopen_s(&fontFile, path.data(), "rb");
            assert(fontFile);
            if (fontFile == NULL)
            {
                assert(!"failed to open font file!");
                return false;
            }

            fseek(fontFile, 0, SEEK_END); /* Set the file pointer to the end of the file and offset 0 byte based on the end of the file */
            size = ftell(fontFile);       /* Get the file size (end of file - head of file, in bytes) */
            fseek(fontFile, 0, SEEK_SET); /* Reset the file pointer to the file header */

            font.fontBuffer = new unsigned char[size * sizeof(unsigned char)];
            fread(font.fontBuffer, size, 1, fontFile);
            fclose(fontFile);

            /* Initialize font */
            if (!stbtt_InitFont(&font.fontInfo, font.fontBuffer, 0))
            {
                assert(!"stb init font failed");
                return false;
            }
        }

        text.fontInfo.create(&iter->second.fontInfo);
        text.fontBuffer.create(&iter->second.fontBuffer);

        Cutlass::TextureInfo ti;
        ti.setSRTex2D(1, 1, true);
        mpContext->createTexture(ti, text.texture);

        {
            Cutlass::BufferInfo bi;
            bi.setVertexBuffer<TextData::RenderingInfo::Vertex>(4);
            mpContext->createBuffer(bi, text.renderingInfo.spriteVB);
        }

        return true;
    }

    void ResourceBank::destroy(TextData& text)
    {
        mpContext->destroyBuffer(text.renderingInfo.spriteVB);
    }

    void ResourceBank::clearCache(std::string_view pathOrName)
    {
        {
            auto&& iter = mModelCacheMap.find(pathOrName.data());
            if (iter != mModelCacheMap.end())
            {
                for (auto& m : iter->second.meshes)
                {
                    mpContext->destroyBuffer(m.VB);
                    mpContext->destroyBuffer(m.IB);
                }

                iter->second.pScene.reset();

                for (auto& t : iter->second.material.textures)
                {
                    mpContext->destroyTexture(t.handle);
                }

                mModelCacheMap.erase(iter);

                return;
            }
        }

        {
            auto&& iter = mSkeletalModelCacheMap.find(pathOrName.data());
            if (iter != mSkeletalModelCacheMap.end())
            {
                for (auto& m : iter->second.meshes)
                {
                    mpContext->destroyBuffer(m.VB);
                    mpContext->destroyBuffer(m.IB);
                }

                iter->second.skeleton.reset();  // explicit
                iter->second.pScene.reset();

                for (auto& t : iter->second.material.textures)
                {
                    mpContext->destroyTexture(t.handle);
                }

                mSkeletalModelCacheMap.erase(iter);

                return;
            }
        }

        {
            auto&& iter = mSpriteCacheMap.find(pathOrName.data());
            if (iter != mSpriteCacheMap.end())
            {
                for (auto& t : iter->second.textures)
                {
                    mpContext->destroyTexture(t);
                }

                mSpriteCacheMap.erase(iter);

                return;
            }
        }

        {
            auto&& iter = mSoundCacheMap.find(pathOrName.data());
            if (iter != mSoundCacheMap.end())
            {
                // Pa_CloseStream(iter->second.pStream);
                mSoundCacheMap.erase(iter);
                return;
            }
        }

        {
            auto&& iter = mFontCacheMap.find(pathOrName.data());
            delete[] iter->second.fontBuffer;

            if (iter != mFontCacheMap.end())
                mFontCacheMap.erase(iter);
            return;
        }
    }

    void ResourceBank::clearCacheAll()
    {
        {
            for (auto& p : mModelCacheMap)
            {
                for (auto& m : p.second.meshes)
                {
                    mpContext->destroyBuffer(m.VB);
                    mpContext->destroyBuffer(m.IB);
                }

                p.second.pScene.reset();

                for (auto& t : p.second.material.textures)
                {
                    mpContext->destroyTexture(t.handle);
                }
            }

            mModelCacheMap.clear();
        }

        {
            for (auto& p : mSkeletalModelCacheMap)
            {
                for (auto& m : p.second.meshes)
                {
                    mpContext->destroyBuffer(m.VB);
                    mpContext->destroyBuffer(m.IB);
                }

                p.second.skeleton.reset();  // explicit
                p.second.pScene.reset();

                for (auto& t : p.second.material.textures)
                {
                    mpContext->destroyTexture(t.handle);
                }
            }

            mSkeletalModelCacheMap.clear();
        }

        {
            for (auto& p : mSpriteCacheMap)
            {
                for (auto& t : p.second.textures)
                {
                    mpContext->destroyTexture(t);
                }
            }

            mSpriteCacheMap.clear();
        }

        {
            // for (auto& p : mSoundCacheMap)
            // {
            //     Pa_CloseStream(p.second.pStream);
            // }

            mSoundCacheMap.clear();
        }

        {
            for (auto& p : mFontCacheMap)
                delete[] p.second.fontBuffer;

            mFontCacheMap.clear();
        }
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
            //?????????????????????????????????
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
                    indices.emplace_back(face.mIndices[j]);
            }
        }

        std::cerr << "indices\n";

        if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
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

            if (skeleton_out.boneMap.count(boneName) <= 0)  //???????????????????????????????????????
            {
                boneIndex = skeleton_out.bones.size();
                skeleton_out.bones.emplace_back();
            }
            else  //?????????
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
            //???????????????
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

                    // assert(k == 3 || !"invalid bone weight!");
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
                Cutlass::Result res = mpContext->createTexture(ti, texture.handle);
                if (res != Cutlass::Result::eSuccess)
                    assert(!"failed to create embedded texture!");
                res = mpContext->writeTexture(embeddedTexture->pcData, texture.handle);
                if (res != Cutlass::Result::eSuccess)
                    assert(!"failed to write to embedded texture!");
            }
            else
            {
                std::string filename = std::regex_replace(path.C_Str(), std::regex("\\\\"), "/");
                filename             = model_out.path.substr(0, model_out.path.find_last_of("/\\")) + '/' + filename;
                // std::wstring filenamews = std::wstring(filename.begin(), filename.end());
                Cutlass::Result res = mpContext->createTextureFromFile(filename.c_str(), texture.handle);
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
