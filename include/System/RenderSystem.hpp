#ifndef MALL_SYSTEM_RENDERSYSTEM_HPP_
#define MALL_SYSTEM_RENDERSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../ComponentData/MaterialData.hpp"
#include "../ComponentData/MeshData.hpp"
#include "../Engine/Graphics.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class RenderSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(RenderSystem, Key, Common)

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
            // ここでCameraに関する情報を拾う

            forEach<MeshData, MaterialData, TransformData>(
                [&](MeshData& mesh, MaterialData& material, TransformData& transform)
                {
                    std::unique_ptr<Graphics>& graphics = common()->graphics;
                    Cutlass::ShaderResourceSet bufferSet, textureSet;

                    {
                        bufferSet.bind(0, )
                    }

                    Cutlass::CommandList cl;
                    cl.begin(graphics->getRenderPass(Graphics::DefaultRenderPass::eDeferred));
                    cl.bind();
                    for (auto& m : mesh.meshes)
                    {
                        cl.bind(m.VB, m.IB);
                        cl.render(m.indices);
                    }

                    cl.end();
                    graphics->writeCommand(Graphics::DefaultRenderPass::eDeferred, cl);
                });
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