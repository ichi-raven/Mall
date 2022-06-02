#include "../../include/Mall/Engine/Graphics.hpp"

#include <algorithm>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace mall
{
    Graphics::Graphics(const std::shared_ptr<Cutlass::Context>& context)
        : mMaxWidth(0)
        , mMaxHeight(0)
        , mpContext(context)
    {
        mpContext->createTextureFromFile("resources/textures/texture.png", mDebugTex);
    }

    Graphics::Graphics(const std::shared_ptr<Cutlass::Context>& context, const std::vector<Cutlass::WindowInfo>& windows)
        : mMaxWidth(0)
        , mMaxHeight(0)
        , mpContext(context)
    {
        assert(windows.size() > 0 || !"no window!");
        for (const auto& window : windows)
            createWindow(window);

        mpContext->createTextureFromFile("resources/textures/texture.png", mDebugTex);
    }

    Graphics::~Graphics()
    {
        std::cerr << "Graphics Engine shut down\n";
    }

    uint32_t Graphics::createWindow(const uint32_t width, const uint32_t height, const char* windowName, bool fullScreen, const uint32_t frameCount, const bool vsync)
    {
        Cutlass::WindowInfo wi(width, height, frameCount, windowName, fullScreen, vsync);
        return createWindow(wi);
    }

    uint32_t Graphics::createWindow(const Cutlass::WindowInfo& wi)
    {
        Window window;

        if (wi.width > mMaxWidth)
            mMaxWidth = wi.width;
        if (wi.height > mMaxHeight)
            mMaxHeight = wi.height;

        window.width  = wi.width;
        window.height = wi.height;

        window.frameCount = wi.frameCount;

        {  // window
            auto&& res = mpContext->createWindow(wi, window.window);
            assert(res == Cutlass::Result::eSuccess || !"failed to create window!");
        }

        {  // g-buffer
            Cutlass::TextureInfo ti;
            ti.setRTTex2DColor(mMaxWidth, mMaxHeight, Cutlass::ResourceType::eF32Vec4);
            auto&& res = mpContext->createTexture(ti, window.gBuffer.albedo);
            assert(res == Cutlass::Result::eSuccess || !"failed to create albedo texture!");
            res = mpContext->createTexture(ti, window.gBuffer.normal);
            assert(res == Cutlass::Result::eSuccess || !"failed to create normal texture!");
            res = mpContext->createTexture(ti, window.gBuffer.worldPos);
            assert(res == Cutlass::Result::eSuccess || !"failed to create worldPos texture!");
            res = mpContext->createTexture(ti, window.gBuffer.metalic);
            assert(res == Cutlass::Result::eSuccess || !"failed to create metalic texture!");
            res = mpContext->createTexture(ti, window.gBuffer.roughness);
            assert(res == Cutlass::Result::eSuccess || !"failed to create roughness texture!");
        }

        {  // final render target
            Cutlass::TextureInfo ti;
            ti.setRTTex2DColor(mMaxWidth, mMaxHeight);
            auto&& res = mpContext->createTexture(ti, window.finalRT);
            assert(res == Cutlass::Result::eSuccess || !"failed to create final render target texture!");
        }

        {  // depth buffer
            Cutlass::TextureInfo ti;
            ti.setRTTex2DDepth(mMaxWidth, mMaxHeight);
            auto&& res = mpContext->createTexture(ti, window.depthBuffer);
            assert(res == Cutlass::Result::eSuccess || !"failed to create depth buffer!");
        }

        {  // initialize each pass

            {  // geometry
                auto& rp = window.geometryPass;
                Cutlass::RenderPassInfo rpi({window.gBuffer.albedo, window.gBuffer.normal, window.gBuffer.worldPos, window.gBuffer.metalic, window.gBuffer.roughness}, window.depthBuffer);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);
            }

            {  // lighting
                auto& rp = window.lightingPass;
                Cutlass::RenderPassInfo rpi(window.finalRT);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;

                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);
            }

            {  // forward
                auto& rp = window.forwardPass;
                Cutlass::RenderPassInfo rpi(window.finalRT, window.depthBuffer, true);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);
            }

            {  // sprite
                auto& rp = window.spritePass;
                Cutlass::RenderPassInfo rpi(window.finalRT, true);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);
            }

            {  // present
                Cutlass::RenderPassInfo rpi(window.window);
                mpContext->createRenderPass(rpi, window.presentPass);

                Cutlass::GraphicsPipelineInfo gpi(
                    Cutlass::Shader("resources/shaders/present/vert.spv", "main"),
                    Cutlass::Shader("resources/shaders/present/frag.spv", "main"),
                    window.presentPass,
                    Cutlass::DepthStencilState::eNone,
                    Cutlass::RasterizerState(Cutlass::PolygonMode::eFill, Cutlass::CullMode::eNone, Cutlass::FrontFace::eClockwise, 1.f),
                    Cutlass::Topology::eTriangleStrip,
                    Cutlass::ColorBlend::eDefault,
                    Cutlass::MultiSampleState::eDefault);

                mpContext->createGraphicsPipeline(gpi, window.presentPipeline);

                Cutlass::ShaderResourceSet SRSet;
                SRSet.bind(0, window.finalRT);
                // SRSet.bind(0, mDebugTex);

                window.presentCommandLists.resize(window.frameCount);
                auto& cls = window.presentCommandLists;

                for (auto& cl : cls)
                {
                    cl.barrier(window.finalRT);
                    cl.begin(window.presentPass, {1.f, 0}, {1.f, 0, 0, 1.f});
                    cl.bind(window.presentPipeline);
                    cl.bind(0, SRSet);
                    // cl.renderImGui();
                    cl.render(4);
                    cl.end();
                }

                auto&& res = mpContext->createCommandBuffer(cls, window.presentCommandBuffer);
                assert(res == Cutlass::Result::eSuccess || !"failed to create present command buffer!");
            }
        }

        mWindows.emplace_back(window);

        return mWindows.size() - 1;
    }

    void Graphics::getWindowSize(uint32_t& width_out, uint32_t& height_out, uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];
        width_out    = window.width;
        height_out   = window.height;
    }

    Cutlass::HBuffer Graphics::createBuffer(const Cutlass::BufferInfo& info)
    {
        Cutlass::HBuffer handle;
        auto&& res = mpContext->createBuffer(info, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to create buffer!");
        return handle;
    }

    void Graphics::destroyBuffer(const Cutlass::HBuffer& handle)
    {
        auto&& res = mpContext->destroyBuffer(handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to destroy buffer!");
    }

    void Graphics::writeBuffer(const size_t size, const void* const pData, const Cutlass::HBuffer& handle)
    {
        assert((size > 0 && pData) || !"invalid writing to buffer memory!");
        auto&& res = mpContext->writeBuffer(size, pData, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to write data to buffer!");
    }

    //テクスチャ作成・破棄
    Cutlass::HTexture Graphics::createTexture(const Cutlass::TextureInfo& info)
    {
        Cutlass::HTexture handle;
        auto&& res = mpContext->createTexture(info, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to create texture!");
        return handle;
    }

    void Graphics::destroyTexture(const Cutlass::HTexture& handle)
    {
        auto&& res = mpContext->destroyTexture(handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to destroy texture!");
    }

    //ファイルからテクスチャ作成
    Cutlass::HTexture Graphics::createTextureFromFile(const char* fileName)
    {
        Cutlass::HTexture handle;
        auto&& res = mpContext->createTextureFromFile(fileName, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to create texture from file!");
        return handle;
    }

    void Graphics::getTextureSize(const Cutlass::HTexture& handle, uint32_t& width_out, uint32_t& height_out, uint32_t& depth_out)
    {
        auto&& res = mpContext->getTextureSize(handle, width_out, height_out, depth_out);
        assert(res == Cutlass::Result::eSuccess || !"failed to get texture size!");
    }

    //テクスチャにデータ書き込み(使用注意, 書き込むデータのサイズはテクスチャのサイズに従うもの以外危険)
    void Graphics::writeTexture(const void* const pData, const Cutlass::HTexture& handle)
    {
        assert(pData || !"invalid writing to buffer memory!");
        auto&& res = mpContext->writeTexture(pData, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to write data to texture!");
    }

    Cutlass::HGraphicsPipeline Graphics::getGraphicsPipeline(
        const Cutlass::GraphicsPipelineInfo& gpi,
        const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        auto&& iter = window.graphicsPipelines.find(gpi);

        if (iter != window.graphicsPipelines.end())
            return iter->second;

        Cutlass::HGraphicsPipeline handle;
        auto&& res = mpContext->createGraphicsPipeline(gpi, handle);
        assert(res == Cutlass::Result::eSuccess || !"failed to create graphics pipeline!");
        window.graphicsPipelines.emplace(gpi, handle);
        return handle;
    }

    const Graphics::GBuffer& Graphics::getGBuffer(const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        return window.gBuffer;
    }

    Cutlass::HRenderPass Graphics::getRenderPass(const DefaultRenderPass passID, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        switch (passID)
        {
            case DefaultRenderPass::eGeometry:
                return window.geometryPass.renderPass;
                break;
            case DefaultRenderPass::eLighting:
                return window.lightingPass.renderPass;
                break;
            case DefaultRenderPass::eForward:
                return window.forwardPass.renderPass;
                break;
            case DefaultRenderPass::eSprite:
                return window.spritePass.renderPass;
                break;
            default:
                assert(!"invalid default render pass!");
                break;
        }
    }

    Cutlass::HRenderPass Graphics::getPrepass(const uint32_t prepassID, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];
        auto iter    = window.prePasses.find(prepassID);
        assert(iter != window.prePasses.end() || !"invalid additional prepass ID!");

        return iter->second.renderPass;
    }

    Cutlass::HRenderPass Graphics::getPostpass(const uint32_t postpassID, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];
        auto iter    = window.postPasses.find(postpassID);
        assert(iter != window.postPasses.end() || !"invalid additional postpass ID!");

        return iter->second.renderPass;
    }

    uint32_t Graphics::addPrepass(const Cutlass::RenderPassInfo& rpi, const uint32_t executionOrder, const uint32_t windowID)
    {
        auto& window = mWindows[windowID];

        assert(window.prePasses.find(executionOrder) == window.prePasses.end() || !"this prepass is already exists!");

        mpContext->createRenderPass(rpi, window.prePasses[executionOrder].renderPass);

        return executionOrder;
    }

    uint32_t Graphics::addPostpass(const Cutlass::RenderPassInfo& rpi, const uint32_t executionOrder, const uint32_t windowID)
    {
        auto& window = mWindows[windowID];

        assert(window.postPasses.find(executionOrder) == window.postPasses.end() || !"this prepass is already exists!");

        mpContext->createRenderPass(rpi, window.postPasses[executionOrder].renderPass);

        return executionOrder;
    }

    void Graphics::writeCommand(const DefaultRenderPass passID, const Cutlass::CommandList& cl, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        Cutlass::Result res = Cutlass::Result::eSuccess;
        switch (passID)
        {
            case DefaultRenderPass::eGeometry:
                res = mpContext->updateCommandBuffer(cl, window.geometryPass.command);
                break;
            case DefaultRenderPass::eLighting:
                res = mpContext->updateCommandBuffer(cl, window.lightingPass.command);
                break;
            case DefaultRenderPass::eForward:
                res = mpContext->updateCommandBuffer(cl, window.forwardPass.command);
                break;
            case DefaultRenderPass::eSprite:
                res = mpContext->updateCommandBuffer(cl, window.spritePass.command);
                break;
            default:
                assert(!"invalid default render pass!");
                break;
        }

        assert(res == Cutlass::Result::eSuccess || !"failed to write command buffer!");
    }

    void Graphics::writeCommandPrepass(const uint32_t prepassID, const Cutlass::CommandList& cl, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        auto&& iter = window.prePasses.find(prepassID);
        assert(iter != window.prePasses.end() || !"invalid prepass ID!");

        mpContext->updateCommandBuffer(cl, iter->second.command);
    }

    void Graphics::writeCommandPostpass(const uint32_t postpassID, const Cutlass::CommandList& cl, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        auto&& iter = window.postPasses.find(postpassID);
        assert(iter != window.postPasses.end() || !"invalid postpass ID!");

        mpContext->updateCommandBuffer(cl, iter->second.command);
    }

    void Graphics::update()
    {
        for (const auto& window : mWindows)
        {
            for (const auto& pass : window.prePasses)
                mpContext->execute(pass.second.command);

            mpContext->execute(window.geometryPass.command);
            mpContext->execute(window.lightingPass.command);
            mpContext->execute(window.forwardPass.command);
            mpContext->execute(window.spritePass.command);

            for (const auto& pass : window.postPasses)
                mpContext->execute(pass.second.command);

            //mpContext->updateCommandBuffer(window.presentCommandLists, window.presentCommandBuffer);
            mpContext->execute(window.presentCommandBuffer);
        }
    }

    bool Graphics::shouldClose()
    {
        return mpContext->shouldClose();
    }

}  // namespace mall