#ifndef MALL_SYSTEM_RENDERSYSTEM_HPP_
#define MALL_SYSTEM_RENDERSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../ComponentData/CameraData.hpp"
#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
#include "../ComponentData/SkeletalMeshData.hpp"
#include "../ComponentData/TransformData.hpp"
#include "../Engine/Graphics.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class RenderSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(RenderSystem, Key, Common)

        struct SceneCB
        {
            SceneCB()
                : world(1.f)
                , view(1.f)
                , proj(1.f)
                , receiveShadow(0)
                , lighting(0)
            {
            }
            glm::mat4 world;
            glm::mat4 view;
            glm::mat4 proj;
            float receiveShadow;
            float lighting;
            glm::vec2 padding;
        };

        constexpr std::size_t MaxBoneNum = 128;
        struct BoneCB
        {
            BoneCB()
                : useBone(0)
            {
            }
            uint useBone;
            glm::vec3 padding;
            glm::mat4x4 boneMat[MaxBoneNum];
        };

        struct Scene2DCB
        {
            Scene2DCB()
                : proj(1.f)
            {
            }

            glm::mat4 proj;
        };

        struct CameraCB
        {
            CameraCB()
                : cameraPos(0)
            {
            }

            glm::vec3 cameraPos;
        };

        constexpr std::size_t MaxLightNum = 16;
        struct LightCB
        {
            LightCB()
                : lightType(0)
                , lightDir(glm::vec3(0, 0, 0))
                , lightColor(glm::vec4(0, 0, 0, 1.f))
                , lightPos(glm::vec3(0, 0, 0))
                , lightRange(0)
            {
            }
            // for packing
            glm::vec3 lightDir;
            uint32_t lightType;
            glm::vec4 lightColor;
            glm::vec3 lightPos;
            float lightRange;
        };

        struct ShadowCB
        {
            ShadowCB()
                : lightViewProj(1.f)
                , lightViewProjBias(1.f)
            {
            }

            glm::mat4 lightViewProj;
            glm::mat4 lightViewProjBias;
        };

    public:
        virtual void onInit()
        {
            std::unique_ptr<Graphics>& graphics = common()->graphics;

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Shader("resources/shaders/deferred/GBuffer_vert.spv"),
                    Shader("resources/shaders/deferred/GBuffer_frag.spv"),
                    graphics->getRenderPass(Graphics::DefaultRenderPass::eGeometry),
                    Cutlass::DepthStencilState,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eBack, Cutlass::FrontFace::eCounterClockwise));

                mGeometryPipeline = graphics->getGraphicsPipeline(gpi);
            }

            {
                Cutlass::GraphicsPipelineInfo gpi(
                    Shader("resources/shaders/deferred/Lighting_vert.spv"),
                    Shader("resources/shaders/deferred/Lighting_frag.spv"),
                    graphics->getRenderPass(Graphics::DefaultRenderPass::eLighting),
                    DepthStencilState::eNone,
                    RasterizerState(PolygonMode::eFill, CullMode::eNone, FrontFace::eClockwise),
                    Topology::eTriangleStrip);

                mLightingPipeline = graphics->getGraphicsPipeline(gpi);
            }
        }

        virtual void onUpdate()
        {
            std::unique_ptr<Graphics>& graphics = common()->graphics;

            SceneCB sceneCB;
            BoneCB boneCB;
            CameraCB cameraCB;
            LightCB lightCB;

            forEach<CameraData, TransformData>(
                [&](CameraData& camera, TransformData& transform)
                {
                    if (camera.enable)
                    {
                        cameraCB.cameraPos = transform.pos;
                        sceneCB.view       = glm::lookAtRH(transform.pos, camera.lookPos, camera.up);
                        sceneCB.proj       = glm::perspective(camera.fovY, camera.aspect, camera.near, camera.far);
                        return;
                    }
                });

            forEach<LightData, TransformData>([&](LightData& light, TransformData& transform)
            {
                
            });

            Cutlass::CommandList cl;
            cl.begin(graphics->getRenderPass(Graphics::DefaultRenderPass::eDeferred));
            forEach<MeshData, MaterialData, TransformData>(
                [&](MeshData& mesh, MaterialData& material, TransformData& transform)
                {
                    Cutlass::ShaderResourceSet bufferSet, textureSet;
                    bufferSet.bind(0, sceneCB);
                    bufferSet.bind(1, boneCB);
                    assert(material.textures.size() > 0 || !"material texture is empty!");
                    textureSet.bind(0, material.textures.begin()->handle);

                    cl.bind();
                    for (auto& m : mesh.meshes)
                    {
                        cl.bind(m.VB, m.IB);
                        cl.render(m.indices);
                    }
                });

            cl.end();
            graphics->writeCommand(Graphics::DefaultRenderPass::eDeferred, cl);
        }

        virtual void onEnd()
        {
        }

    private:
        Cutlass::HGraphicsPipeline mGeometryPipeline;
        Cutlass::HGraphicsPipeline mLightingPipeline;
    };
}  // namespace mall

#endif