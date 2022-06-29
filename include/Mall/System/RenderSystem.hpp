#ifndef MALL_SYSTEM_RENDERSYSTEM_HPP_
#define MALL_SYSTEM_RENDERSYSTEM_HPP_

#include <Cutlass/Cutlass.hpp>
#include <MVECS/ISystem.hpp>
#include <MVECS/World.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "../ComponentData/CameraData.hpp"
#include "../ComponentData/LightData.hpp"
#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
#include "../ComponentData/RigidBodyData.hpp"
#include "../ComponentData/SkeletalMeshData.hpp"
#include "../ComponentData/TransformData.hpp"
#include "../Engine.hpp"
#include "../Engine/Graphics.hpp"

namespace mall
{
    namespace SFINAE
    {
        struct HasGraphicsImpl
        {
            template <class T>
            static std::true_type check(decltype(T::graphics)*);

            template <class T>
            static std::false_type check(...);
        };

        template <class T>
        class HasGraphics : public decltype(HasGraphicsImpl::check<T>(nullptr))
        {
        };

        template <typename T>
        constexpr bool HasGraphicsValue = HasGraphics<T>();

        template <typename Common>
        constexpr bool IsEngineCommon = std::is_base_of_v<Engine, Common>&& SFINAE::HasGraphicsValue<Common>;

    };  // namespace SFINAE

    template <typename Key, typename Common, typename = std::enable_if_t<SFINAE::IsEngineCommon<Common>>>
    class RenderSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(RenderSystem, Key, Common)

    public:
        virtual void onInit()
        {
            std::unique_ptr<Graphics>& graphics = this->common().graphics;

            mGeometryPass = graphics->getRenderPass(Graphics::DefaultRenderPass::eGeometry);
            mLightingPass = graphics->getRenderPass(Graphics::DefaultRenderPass::eLighting);
            mSpritePass = graphics->getRenderPass(Graphics::DefaultRenderPass::eSprite);


            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/deferred/GBuffer_vert.spv"),
                    Cutlass::Shader("resources/shaders/deferred/GBuffer_frag.spv"),
                    mGeometryPass,
                    Cutlass::DepthStencilState::eDepth,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eBack, Cutlass::FrontFace::eCounterClockwise));

                mGeometryPipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/deferred/Lighting_vert.spv"),
                    Cutlass::Shader("resources/shaders/deferred/Lighting_frag.spv"),
                    mLightingPass,
                    Cutlass::DepthStencilState::eNone,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eNone, Cutlass::FrontFace::eClockwise),
                    Cutlass::Topology::eTriangleStrip);

                mLightingPipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/sprite/Sprite_vert.spv"),
                    Cutlass::Shader("resources/shaders/sprite/Sprite_frag.spv"),
                    mSpritePass,
                    Cutlass::DepthStencilState::eNone,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eNone, Cutlass::FrontFace::eClockwise),
                    Cutlass::Topology::eTriangleList,
                    Cutlass::ColorBlend::eAlphaBlend);

                mSpritePipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::BufferInfo bi;

                // this->template forEach<MeshData>(
                //     [&](MeshData& mesh)
                //     {
                //         bi.setUniformBuffer<MeshData::RenderingInfo::SceneCBParam>();
                //         mesh.renderingInfo.sceneCB = graphics->createBuffer(bi);
                //     });

                // this->template forEach<SkeletalMeshData>(
                //     [&](SkeletalMeshData& skeletalMesh)
                //     {
                //         bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::SceneCBParam>();
                //         skeletalMesh.renderingInfo.sceneCB = graphics->createBuffer(bi);

                //         bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::BoneCBParam>();
                //         skeletalMesh.renderingInfo.boneCB = graphics->createBuffer(bi);
                //     });

                // this->template forEach<SpriteData>(
                //     [&](SpriteData& sprite)
                //     {
                //         bi.setVertexBuffer<SpriteData::RenderingInfo::Vertex>(4);
                //         sprite.renderingInfo.spriteVB = graphics->createBuffer(bi);
                //     });

                // this->template forEach<TextData>(
                //     [&](TextData& text)
                //     {
                //         bi.setVertexBuffer<TextData::RenderingInfo::Vertex>(4);
                //         text.renderingInfo.spriteVB = graphics->createBuffer(bi);
                //     });

                bi.setUniformBuffer<CameraData::RenderingInfo::CameraCBParam>();
                mCameraCB = graphics->createBuffer(bi);

                bi.setUniformBuffer<LightData::RenderingInfo::LightCBParam>(LightData::RenderingInfo::MaxLightNum);
                mLightCB = graphics->createBuffer(bi);

                bi.setUniformBuffer<LightData::RenderingInfo::ShadowCBParam>(LightData::RenderingInfo::MaxLightNum);
                mShadowCB = graphics->createBuffer(bi);

                bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::BoneCBParam>();
                mDummyBoneCB = graphics->createBuffer(bi);
                {
                    SkeletalMeshData::RenderingInfo::BoneCBParam param;
                    for (std::size_t i = 0; i < SkeletalMeshData::RenderingInfo::MaxBoneNum; ++i)
                        param.boneMat[i] = glm::mat4(1.f);
                    graphics->writeBuffer(sizeof(SkeletalMeshData::RenderingInfo::BoneCBParam), &param, mDummyBoneCB);
                }

                {
                    std::array<uint32_t, 6> indices =
                        {{0, 2, 1, 1, 2, 3}};

                    Cutlass::BufferInfo bi;
                    bi.setIndexBuffer<uint32_t>(6);
                    mSpriteIB = graphics->createBuffer(bi);
                    graphics->writeBuffer(6 * sizeof(uint32_t), indices.data(), mSpriteIB);

                    std::uint32_t width, height;
                    graphics->getWindowSize(width, height);
                    SpriteData::RenderingInfo::Scene2DCBParam scene2DCBParam;
                    scene2DCBParam.proj =
                        {
                            2.f / width, 0.f, 0.f, 0,
                            0.f, 2.f / height, 0.f, 0,
                            0.f, 0.f, 1.f, 0.f,
                            -1.f, -1.f, 0.f, 1.f};

                    bi.setUniformBuffer<SpriteData::RenderingInfo::Scene2DCBParam>();
                    mSpriteCB = graphics->createBuffer(bi);
                    graphics->writeBuffer(sizeof(SpriteData::RenderingInfo::Scene2DCBParam), &scene2DCBParam, mSpriteCB);
                }
            }

            {  // lighting pass
                Cutlass::ShaderResourceSet bufferSet, textureSet;
                Cutlass::CommandList cl;

                bufferSet.bind(0, mLightCB);
                bufferSet.bind(1, mCameraCB);

                auto& gBuffer = graphics->getGBuffer();
                textureSet.bind(0, gBuffer.albedo);
                textureSet.bind(1, gBuffer.normal);
                textureSet.bind(2, gBuffer.worldPos);
                // metalicとroughnessつける

                cl.barrier(gBuffer.albedo);
                cl.barrier(gBuffer.normal);
                cl.barrier(gBuffer.worldPos);
                cl.barrier(gBuffer.metalic);
                cl.barrier(gBuffer.roughness);

                cl.begin(mLightingPass, {1.f, 0}, {1.f, 0, 0, 1.f});
                cl.bind(mLightingPipeline);
                cl.bind(0, bufferSet);
                cl.bind(1, textureSet);
                cl.render(4, 1, 0, 0);
                cl.end();
                graphics->writeCommand(Graphics::DefaultRenderPass::eLighting, cl);
            }
        }

        virtual void onUpdate()
        {
            std::unique_ptr<Graphics>& graphics = this->common().graphics;

            static MeshData::RenderingInfo::SceneCBParam meshSceneCBParam;
            static SkeletalMeshData::RenderingInfo::SceneCBParam skeletalSceneCBParam;
            static SkeletalMeshData::RenderingInfo::BoneCBParam boneCBParam;
            static CameraData::RenderingInfo::CameraCBParam cameraCBParam;
            static LightData::RenderingInfo::LightCBParam lightCBParam[LightData::RenderingInfo::MaxLightNum];
            // static LightData::RenderingInfo::ShadowCBParam shadowCBParam;

            {
                std::function<void(CameraData&, TransformData&)> f =
                    [&](CameraData& camera, TransformData& transform)
                {
                    if (camera.enable)
                    {
                        cameraCBParam.cameraPos = transform.pos;
                        auto view               = glm::lookAtRH(transform.pos, camera.lookPos, camera.up);
                        auto&& proj             = glm::perspective(camera.fovY, camera.aspect, camera.near, camera.far);
                        view[1][1] *= -1.f;
                        meshSceneCBParam.view     = view;
                        meshSceneCBParam.proj     = proj;
                        skeletalSceneCBParam.view = view;
                        skeletalSceneCBParam.proj = proj;
                    }
                };

                this->template forEach<CameraData, TransformData>(f);
            }

            {
                std::size_t lightCount = 0;

                static std::function<void(LightData&, TransformData&)> f =
                    [&](LightData& light, TransformData& transform)
                {
                    if (lightCount >= LightData::RenderingInfo::MaxLightNum)
                        return;

                    lightCBParam[lightCount].lightColor = light.color;
                    switch (light.type)
                    {
                        case LightData::LightType::eDirectional:
                            lightCBParam[lightCount].lightType = static_cast<std::uint32_t>(LightData::LightType::eDirectional);
                            lightCBParam[lightCount].lightDir  = light.direction;
                            break;
                        case LightData::LightType::ePoint:
                            lightCBParam[lightCount].lightType  = static_cast<std::uint32_t>(LightData::LightType::ePoint);
                            lightCBParam[lightCount].lightPos   = transform.pos;
                            lightCBParam[lightCount].lightRange = light.range;
                            break;
                    }

                    ++lightCount;
                };

                this->template forEach<LightData, TransformData>(f);
            }

            graphics->writeBuffer(sizeof(LightData::RenderingInfo::LightCBParam) * LightData::RenderingInfo::MaxLightNum, &lightCBParam, mLightCB);

            Cutlass::CommandList cl;
            bool debug = false;
            //cl.begin(mGeometryPass, {1.f, 0}, {0.2f, 0.2f, 0.2f, 0});
            cl.clear();
            cl.begin(mGeometryPass);

            {  // mesh
                std::function<void(MeshData&, MaterialData&)> f =
                    [&](MeshData& mesh, MaterialData& material)
                {
                    meshSceneCBParam.world         = mesh.world * mesh.defaultAxis;
                    meshSceneCBParam.lighting      = 1;
                    meshSceneCBParam.receiveShadow = 0;
                    meshSceneCBParam.useBone       = 0;

                    graphics->writeBuffer(sizeof(MeshData::RenderingInfo::SceneCBParam), &meshSceneCBParam, mesh.renderingInfo.sceneCB);

                    Cutlass::ShaderResourceSet bufferSet, textureSet;

                    bufferSet.bind(0, mesh.renderingInfo.sceneCB);
                    bufferSet.bind(1, mDummyBoneCB);
                    assert(material.textures.size() > 0 || !"material texture is empty!");
                    textureSet.bind(0, material.textures.begin()->handle);

                    cl.bind(mGeometryPipeline);
                    cl.bind(0, bufferSet);
                    for (std::size_t i = 0; i < mesh.meshes.size(); ++i)
                    {
                        auto& m = mesh.meshes[i];
                        textureSet.bind(0, material.textures[i].handle);
                        cl.bind(1, textureSet);
                        cl.bind(m.VB, m.IB);
                        cl.renderIndexed(m.indices.size());
                        debug = true;
                    }
                };

                this->template forEach<MeshData, MaterialData>(f);
            }

            {  // skeletal mesh
                std::function<void(SkeletalMeshData&, MaterialData&)> f =
                    [&](SkeletalMeshData& mesh, MaterialData& material)
                {
                    skeletalSceneCBParam.world         = mesh.world * mesh.defaultAxis;
                    skeletalSceneCBParam.lighting      = 1;
                    skeletalSceneCBParam.receiveShadow = 0;
                    skeletalSceneCBParam.useBone       = 1;

                    for (std::size_t i = 0; i < SkeletalMeshData::RenderingInfo::MaxBoneNum; ++i)
                    {
                        if (i >= mesh.skeleton.get().bones.size())
                            boneCBParam.boneMat[i] = glm::mat4(1.f);
                        else
                            boneCBParam.boneMat[i] = mesh.skeleton.get().bones[i].transform;
                    }

                    graphics->writeBuffer(sizeof(SkeletalMeshData::RenderingInfo::SceneCBParam), &skeletalSceneCBParam, mesh.renderingInfo.sceneCB);
                    graphics->writeBuffer(sizeof(SkeletalMeshData::RenderingInfo::BoneCBParam), &boneCBParam, mesh.renderingInfo.boneCB);

                    Cutlass::ShaderResourceSet bufferSet, textureSet;

                    bufferSet.bind(0, mesh.renderingInfo.sceneCB);
                    bufferSet.bind(1, mesh.renderingInfo.boneCB);
                    assert(material.textures.size() > 0 || !"material texture is empty!");

                    cl.bind(mGeometryPipeline);
                    cl.bind(0, bufferSet);
                    for (std::size_t i = 0; i < mesh.meshes.size(); ++i)
                    {
                        auto& m = mesh.meshes[i];
                        cl.bind(m.VB, m.IB);
                        textureSet.bind(0, material.textures[i].handle);
                        cl.bind(1, textureSet);
                        cl.renderIndexed(m.indices.size());
                        debug = true;
                    }
                };

                this->template forEach<SkeletalMeshData, MaterialData>(f);
            }

            cl.end();
            // for (auto& cmd : cl.getInternalCommandData())
            // {
            //     std::cerr << static_cast<int>(cmd.first) << "\n";
            // }
            //std::cerr << "A UB : " << cl.getUniformBufferCount() << ", CT : " << cl.getCombinedTextureCount() << "\n";
            //if (debug)
                graphics->writeCommand(Graphics::DefaultRenderPass::eGeometry, cl);

            {  // sprite

                glm::vec3 lu(0), ld(0), ru(0), rd(0);
                Cutlass::ShaderResourceSet bufferSet, textureSet;
                bufferSet.bind(0, mSpriteCB);

                cl.clear();
                cl.begin(mSpritePass, {1.f, 0}, {0.2f, 0.2f, 0.2f, 0});
                {
                    std::function<void(SpriteData&, TransformData&)> f =
                        [&](SpriteData& sprite, TransformData& transform)
                    {
                        {  // 頂点更新
                            lu = ld = ru = rd = glm::vec3(0);
                            const auto& scale = transform.scale;
                            const auto& rot   = transform.rot;
                            const auto& pos   = transform.pos;
                            glm::uvec2 size;
                            std::uint32_t depth;
                            graphics->getTextureSize(sprite.textures[sprite.index], size.x, size.y, depth);
                            assert(depth == 1);

                            if (sprite.centerFlag)
                            {
                                rd.x = ru.x = 1.f * scale.x * size.x / 2.f;
                                ld.y = rd.y = 1.f * scale.y * size.y / 2.f;
                                lu.x = ld.x = -1.f * scale.x * size.x / 2.f;
                                lu.y = ru.y = -1.f * scale.y * size.y / 2.f;
                                lu          = rot * lu;
                            }
                            else
                            {
                                rd.x = ru.x = size.x * scale.x;
                                rd.y = ld.y = size.y * scale.y;
                            }

                            ld = rot * ld;
                            ru = rot * ru;
                            rd = rot * rd;
                            lu += pos;
                            ld += pos;
                            ru += pos;
                            rd += pos;
                            lu.z = std::min(std::max(0.f, pos.z), 1.f);
                            ld.z = std::min(std::max(0.f, pos.z), 1.f);
                            ru.z = std::min(std::max(0.f, pos.z), 1.f);
                            rd.z = std::min(std::max(0.f, pos.z), 1.f);

                            // std::cerr << "sprite pos : " << glm::to_string(lu) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(ld) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(ru) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(rd) << "\n";

                            //ここでスプライトの頂点位置を更新する
                            std::array<SpriteData::RenderingInfo::Vertex, 4> vertices =
                                {{
                                    {lu, glm::vec2(0, 0)},
                                    {ru, glm::vec2(1.f, 0)},
                                    {ld, glm::vec2(0, 1.f)},
                                    {rd, glm::vec2(1.f, 1.f)},
                                }};

                            //頂点バッファ構築
                            graphics->writeBuffer(4 * sizeof(SpriteData::RenderingInfo::Vertex), vertices.data(), sprite.renderingInfo.spriteVB);
                        }

                        textureSet.bind(0, sprite.textures[sprite.index]);
                        sprite.index = (1 + sprite.index) % sprite.textures.size();

                        cl.bind(mSpritePipeline);
                        cl.bind(sprite.renderingInfo.spriteVB, mSpriteIB);
                        cl.bind(0, bufferSet);
                        cl.bind(1, textureSet);

                        cl.renderIndexed(6);
                    };

                    this->template forEach<mall::SpriteData, mall::TransformData>(f);
                }

                {
                    std::function<void(TextData&, TransformData&)> f =
                        [&](TextData& text, TransformData& transform)
                    {
                        {  // 頂点更新
                            lu = ld = ru = rd = glm::vec3(0);
                            const auto& scale = transform.scale;
                            const auto& rot   = transform.rot;
                            const auto& pos   = transform.pos;
                            glm::uvec2 size;
                            std::uint32_t depth;
                            graphics->getTextureSize(text.texture, size.x, size.y, depth);
                            assert(depth == 1);

                            if (text.centerFlag)
                            {
                                rd.x = ru.x = 1.f * scale.x * size.x / 2.f;
                                ld.y = rd.y = 1.f * scale.y * size.y / 2.f;
                                lu.x = ld.x = -1.f * scale.x * size.x / 2.f;
                                lu.y = ru.y = -1.f * scale.y * size.y / 2.f;
                                lu          = rot * lu;
                            }
                            else
                            {
                                rd.x = ru.x = size.x * scale.x;
                                rd.y = ld.y = size.y * scale.y;
                            }

                            ld = rot * ld;
                            ru = rot * ru;
                            rd = rot * rd;
                            lu += pos;
                            ld += pos;
                            ru += pos;
                            rd += pos;
                            lu.z = std::min(std::max(0.f, pos.z), 1.f);
                            ld.z = std::min(std::max(0.f, pos.z), 1.f);
                            ru.z = std::min(std::max(0.f, pos.z), 1.f);
                            rd.z = std::min(std::max(0.f, pos.z), 1.f);

                            // std::cerr << "sprite pos : " << glm::to_string(lu) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(ld) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(ru) << "\n";
                            // std::cerr << "sprite pos : " << glm::to_string(rd) << "\n";

                            std::array<TextData::RenderingInfo::Vertex, 4> vertices =
                                {{
                                    {lu, glm::vec2(0, 0)},
                                    {ru, glm::vec2(1.f, 0)},
                                    {ld, glm::vec2(0, 1.f)},
                                    {rd, glm::vec2(1.f, 1.f)},
                                }};

                            //頂点バッファ構築
                            graphics->writeBuffer(4 * sizeof(TextData::RenderingInfo::Vertex), vertices.data(), text.renderingInfo.spriteVB);
                        }

                        textureSet.bind(0, text.texture);

                        cl.bind(mSpritePipeline);
                        cl.bind(text.renderingInfo.spriteVB, mSpriteIB);
                        cl.bind(0, bufferSet);
                        cl.bind(1, textureSet);

                        cl.renderIndexed(6);
                    };

                    this->template forEach<TextData, TransformData>(f);
                }

                cl.end();
                graphics->writeCommand(Graphics::DefaultRenderPass::eSprite, cl);
            }
        }

        virtual void onEnd()
        {
            std::unique_ptr<Graphics>& graphics = this->common().graphics;
            graphics->destroyBuffer(mLightCB);
            graphics->destroyBuffer(mShadowCB);
            graphics->destroyBuffer(mCameraCB);
            graphics->destroyBuffer(mDummyBoneCB);
            graphics->destroyBuffer(mSpriteIB);

            // this->template forEach<MeshData>(
            //     [&](MeshData& mesh)
            //     {
            //         graphics->destroyBuffer(mesh.renderingInfo.sceneCB);
            //     });

            // this->template forEach<SkeletalMeshData>(
            //     [&](SkeletalMeshData& skeletalMesh)
            //     {
            //         graphics->destroyBuffer(skeletalMesh.renderingInfo.sceneCB);
            //         graphics->destroyBuffer(skeletalMesh.renderingInfo.boneCB);
            //     });

            // this->template forEach<SpriteData>(
            //     [&](SpriteData& sprite)
            //     {
            //         graphics->destroyBuffer(sprite.renderingInfo.spriteVB);
            //     });

            // this->template forEach<TextData>(
            //     [&](TextData& text)
            //     {
            //         graphics->destroyBuffer(text.renderingInfo.spriteVB);
            //     });

        }

    protected:
        Cutlass::HRenderPass mGeometryPass;
        Cutlass::HRenderPass mLightingPass;
        Cutlass::HRenderPass mSpritePass;

        Cutlass::HGraphicsPipeline mGeometryPipeline;
        Cutlass::HGraphicsPipeline mLightingPipeline;
        Cutlass::HGraphicsPipeline mSpritePipeline;

        Cutlass::HBuffer mLightCB;
        Cutlass::HBuffer mShadowCB;
        Cutlass::HBuffer mDummyBoneCB;
        Cutlass::HBuffer mCameraCB;
        Cutlass::HBuffer mSpriteIB;
        Cutlass::HBuffer mSpriteCB;
    };
}  // namespace mall

#endif