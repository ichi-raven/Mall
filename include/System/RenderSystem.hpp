#ifndef MALL_SYSTEM_RENDERSYSTEM_HPP_
#define MALL_SYSTEM_RENDERSYSTEM_HPP_

#include <Cutlass/Cutlass.hpp>
#include <MVECS/ISystem.hpp>
#include <MVECS/World.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "../ComponentData/CameraData.hpp"
#include "../ComponentData/LightData.hpp"
#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
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

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/deferred/GBuffer_vert.spv"),
                    Cutlass::Shader("resources/shaders/deferred/GBuffer_frag.spv"),
                    graphics->getRenderPass(Graphics::DefaultRenderPass::eGeometry),
                    Cutlass::DepthStencilState::eDepth,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eBack, Cutlass::FrontFace::eCounterClockwise));

                mGeometryPipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/deferred/Lighting_vert.spv"),
                    Cutlass::Shader("resources/shaders/deferred/Lighting_frag.spv"),
                    graphics->getRenderPass(Graphics::DefaultRenderPass::eLighting),
                    Cutlass::DepthStencilState::eNone,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eNone, Cutlass::FrontFace::eCounterClockwise),
                    Cutlass::Topology::eTriangleStrip);

                mLightingPipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::BufferInfo bi;

                this->template forEach<MeshData>(
                    [&](MeshData& mesh)
                    {
                        bi.setUniformBuffer<MeshData::RenderingInfo::SceneCBParam>();
                        mesh.renderingInfo.sceneCB = graphics->createBuffer(bi);
                    });

                this->template forEach<SkeletalMeshData>(
                    [&](SkeletalMeshData& skeletalMesh)
                    {
                        bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::SceneCBParam>();
                        skeletalMesh.renderingInfo.sceneCB = graphics->createBuffer(bi);

                        bi.setUniformBuffer<SkeletalMeshData::RenderingInfo::BoneCBParam>();
                        skeletalMesh.renderingInfo.boneCB = graphics->createBuffer(bi);
                    });

                this->template forEach<SpriteData>(
                    [&](SpriteData& sprite)
                    {
                        bi.setUniformBuffer<SpriteData::RenderingInfo::Scene2DCBParam>();
                        sprite.renderingInfo.scene2DCB = graphics->createBuffer(bi);
                    });

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
            }

            {
                Cutlass::ShaderResourceSet bufferSet, textureSet;
                Cutlass::CommandList cl;

                bufferSet.bind(0, mLightCB);
                bufferSet.bind(1, mCameraCB);

                auto& gBuffer = graphics->getGBuffer();
                textureSet.bind(0, gBuffer.albedo);
                textureSet.bind(1, gBuffer.normal);
                textureSet.bind(2, gBuffer.worldPos);
                // metalicとroughnessつける

                cl.begin(graphics->getRenderPass(Graphics::DefaultRenderPass::eLighting));
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
                        cameraCBParam.cameraPos   = transform.pos;
                        auto view                 = glm::lookAtRH(transform.pos, camera.lookPos, camera.up);
                        auto proj                 = glm::perspective(camera.fovY, camera.aspect, camera.near, camera.far);
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
                            lightCBParam[lightCount].lightType = 0;
                            lightCBParam[lightCount].lightDir  = light.direction;
                            break;
                        case LightData::LightType::ePoint:
                            lightCBParam[lightCount].lightType  = 1;
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

            auto& gBuffer = graphics->getGBuffer();

            cl.barrier(gBuffer.albedo);
            cl.barrier(gBuffer.normal);
            cl.barrier(gBuffer.worldPos);
            cl.barrier(gBuffer.metalic);
            cl.barrier(gBuffer.roughness);
            cl.begin(graphics->getRenderPass(Graphics::DefaultRenderPass::eGeometry));

            {
                std::function<void(MeshData&, MaterialData&, TransformData&)> f =
                    [&](MeshData& mesh, MaterialData& material, TransformData& transform)
                {
                    meshSceneCBParam.world         = glm::translate(glm::mat4(1.f), transform.pos) * glm::toMat4(transform.rotation) * glm::scale(transform.scale);
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
                    cl.bind(1, textureSet);
                    for (auto& m : mesh.meshes)
                    {
                        cl.bind(m.VB, m.IB);
                        cl.renderIndexed(m.indices.size());
                    }
                };

                this->template forEach<MeshData, MaterialData, TransformData>(f);
            }

            {
                std::function<void(SkeletalMeshData&, MaterialData&, TransformData&)> f =
                    [&](SkeletalMeshData& mesh, MaterialData& material, TransformData& transform)
                {
                    skeletalSceneCBParam.world         = glm::translate(glm::mat4(1.f), transform.pos) * glm::toMat4(transform.rotation) * glm::scale(transform.scale);
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
                    textureSet.bind(0, material.textures.begin()->handle);

                    cl.bind(mGeometryPipeline);
                    cl.bind(0, bufferSet);
                    cl.bind(1, textureSet);
                    for (auto& m : mesh.meshes)
                    {
                        cl.bind(m.VB, m.IB);
                        cl.renderIndexed(m.indices.size());
                    }
                };

                this->template forEach<SkeletalMeshData, MaterialData, TransformData>(f);
            }

            cl.end();
            graphics->writeCommand(Graphics::DefaultRenderPass::eGeometry, cl);
        }

        virtual void onEnd()
        {
            std::unique_ptr<Graphics>& graphics = this->common().graphics;
            graphics->destroyBuffer(mLightCB);
            graphics->destroyBuffer(mShadowCB);
            graphics->destroyBuffer(mCameraCB);
        }

    protected:
        Cutlass::HGraphicsPipeline mGeometryPipeline;
        Cutlass::HGraphicsPipeline mLightingPipeline;

        Cutlass::HBuffer mLightCB;
        Cutlass::HBuffer mShadowCB;
        Cutlass::HBuffer mDummyBoneCB;
        Cutlass::HBuffer mCameraCB;
    };
}  // namespace mall

#endif