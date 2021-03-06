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
                //auto& rp = window.geometryPass;
                RenderPass rp;
                rp.passName = std::string("geometry");
                Cutlass::RenderPassInfo rpi({window.gBuffer.albedo, window.gBuffer.normal, window.gBuffer.worldPos, window.gBuffer.metalic, window.gBuffer.roughness}, window.depthBuffer);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);

                window.insertRenderPass(static_cast<int>(DefaultRenderPass::eGeometry), rp);
            }

            {  // lighting
                //auto& rp = window.lightingPass;
                RenderPass rp;
                rp.passName = std::string("lighting");
                Cutlass::RenderPassInfo rpi(window.finalRT);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;

                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);

                window.insertRenderPass(static_cast<int>(DefaultRenderPass::eLighting), rp);
            }

            {  // forward
                //auto& rp = window.forwardPass;
                RenderPass rp;
                rp.passName = std::string("forward");
                Cutlass::RenderPassInfo rpi(window.finalRT, window.depthBuffer, true);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);

                window.insertRenderPass(static_cast<int>(DefaultRenderPass::eForward), rp);
            }

            {  // sprite
                //auto& rp = window.spritePass;
                RenderPass rp;
                rp.passName = std::string("sprite");
                Cutlass::RenderPassInfo rpi(window.finalRT, true);
                mpContext->createRenderPass(rpi, rp.renderPass);

                Cutlass::CommandList cl;
                cl.begin(rp.renderPass);
                cl.end();
                mpContext->createCommandBuffer(cl, rp.command);

                window.insertRenderPass(static_cast<int>(DefaultRenderPass::eSprite), rp);
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

    //??????????????????????????????
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

    //???????????????????????????????????????
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

    //???????????????????????????????????????(????????????, ??????????????????????????????????????????????????????????????????????????????????????????)
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

    int Graphics::getExecutionOrder(const DefaultRenderPass passID) const
    {
        return static_cast<int>(passID);
    }

    Cutlass::HRenderPass Graphics::getRenderPass(const DefaultRenderPass passID, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        switch (passID)
        {
            case DefaultRenderPass::eGeometry:
                return window.renderPasses[window.geometryPassIndex].second.renderPass;
                //std::cerr << "geom\n";
                break;
            case DefaultRenderPass::eLighting:
                return window.renderPasses[window.lightingPassIndex].second.renderPass;
                //std::cerr << "light\n";
                break;
            case DefaultRenderPass::eForward:
                return window.renderPasses[window.forwardPassIndex].second.renderPass;
                //std::cerr << "forward\n";
                break;
            case DefaultRenderPass::eSprite:
                return window.renderPasses[window.spritePassIndex].second.renderPass;
                //std::cerr << "sprite\n";
                break;
            default:
                assert(!"invalid default render pass!");
                break;
        }
    }

    Cutlass::HRenderPass Graphics::getRenderPass(const int executionOrder, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        assert(executionOrder >= window.renderPasses.size() || !"invalid execution order!");

        return window.renderPasses[window.findRenderPass(executionOrder)].second.renderPass;
    }

    void Graphics::addRenderPass(const Cutlass::RenderPassInfo& rpi, const int executionOrder, std::string_view passName, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        RenderPass renderPass;

        renderPass.passName = std::string(passName);

        mpContext->createRenderPass(rpi, renderPass.renderPass);

        Cutlass::CommandList cl;
        cl.begin(renderPass.renderPass);
        cl.end();

        mpContext->createCommandBuffer(cl, renderPass.command);

        window.insertRenderPass(executionOrder, renderPass);

        // ????????????????????????????????????????????????????????????????????????
        if (executionOrder <= static_cast<int>(DefaultRenderPass::eSprite))
        {
            ++window.spritePassIndex;
            if (executionOrder <= static_cast<int>(DefaultRenderPass::eForward))
            {
                ++window.forwardPassIndex;
                if (executionOrder <= static_cast<int>(DefaultRenderPass::eLighting))
                {
                    ++window.lightingPassIndex;
                    if (executionOrder <= static_cast<int>(DefaultRenderPass::eGeometry))
                        ++window.geometryPassIndex;
                }
            }   
        }
    }

    std::pair<int, Cutlass::HRenderPass> Graphics::findRenderPass(std::string_view passName, const uint32_t windowID) const
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        for (const auto& pass : window.renderPasses)
            if (pass.second.passName == passName)
                return std::make_pair(pass.first, pass.second.renderPass);
    }

    /*Cutlass::HRenderPass Graphics::getPrepass(const uint32_t prepassID, const uint32_t windowID) const
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
    }*/

    void Graphics::writeCommand(const DefaultRenderPass passID, const Cutlass::CommandList& cl, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        Cutlass::Result res = Cutlass::Result::eSuccess;
        switch (passID)
        {
            case DefaultRenderPass::eGeometry:
                res = mpContext->updateCommandBuffer(cl, window.renderPasses[window.geometryPassIndex].second.command);
                //std::cerr << "geom\n";
                break;
            case DefaultRenderPass::eLighting:
                res = mpContext->updateCommandBuffer(cl, window.renderPasses[window.lightingPassIndex].second.command);
                //std::cerr << "light\n";
                break;
            case DefaultRenderPass::eForward:
                res = mpContext->updateCommandBuffer(cl, window.renderPasses[window.forwardPassIndex].second.command);
                //std::cerr << "forward\n";
                break;
            case DefaultRenderPass::eSprite:
                res = mpContext->updateCommandBuffer(cl, window.renderPasses[window.spritePassIndex].second.command);
                //std::cerr << "sprite\n";
                break;
            default:
                assert(!"invalid default render pass!");
                break;
        }

        assert(res == Cutlass::Result::eSuccess || !"failed to write command buffer!");
    }

    void Graphics::writeCommand(const int executionOrder, const Cutlass::CommandList& cl, const uint32_t windowID)
    {
        assert(windowID < mWindows.size() || !"invalid window ID!");
        auto& window = mWindows[windowID];

        std::size_t index = window.findRenderPass(executionOrder);
        assert(index < window.renderPasses.size() || !"the renderpass that have this execution order is not registered");

        auto res = mpContext->updateCommandBuffer(cl, window.renderPasses[index].second.command);
        assert(res == Cutlass::Result::eSuccess || !"failed to write command buffer!");
    }


    //void Graphics::writeCommandPrepass(const uint32_t prepassID, const Cutlass::CommandList& cl, const uint32_t windowID)
    //{
    //    assert(windowID < mWindows.size() || !"invalid window ID!");
    //    auto& window = mWindows[windowID];

    //    auto&& iter = window.prePasses.find(prepassID);
    //    assert(iter != window.prePasses.end() || !"invalid prepass ID!");

    //    mpContext->updateCommandBuffer(cl, iter->second.command);
    //}

    //void Graphics::writeCommandPostpass(const uint32_t postpassID, const Cutlass::CommandList& cl, const uint32_t windowID)
    //{
    //    assert(windowID < mWindows.size() || !"invalid window ID!");
    //    auto& window = mWindows[windowID];

    //    auto&& iter = window.postPasses.find(postpassID);
    //    assert(iter != window.postPasses.end() || !"invalid postpass ID!");

    //    mpContext->updateCommandBuffer(cl, iter->second.command);
    //}

    void Graphics::update()
    {
        for (const auto& window : mWindows)
        {
            //for (const auto& pass : window.prePasses)
            //    mpContext->execute(pass.second.command);

            //mpContext->execute(window.geometryPass.command);
            ////std::cerr << "geom\n";
            //mpContext->execute(window.lightingPass.command);
            ////std::cerr << "light\n";
            //mpContext->execute(window.forwardPass.command);
            ////std::cerr << "forward\n";
            //mpContext->execute(window.spritePass.command);
            ////std::cerr << "sprite\n";

            //for (const auto& pass : window.postPasses)
            //    mpContext->execute(pass.second.command);

            for (const auto& pass : window.renderPasses)
                mpContext->execute(pass.second.command);

            //mpContext->updateCommandBuffer(window.presentCommandLists, window.presentCommandBuffer);
            mpContext->execute(window.presentCommandBuffer);
            //std::cerr << "present\n";
        }
    }

    bool Graphics::shouldClose()
    {
        return mpContext->shouldClose();
    }

    Graphics::RenderPass& Graphics::Window::insertRenderPass(int executionOrder, const RenderPass& renderPass)
    {
        auto&& pair = std::pair<int, RenderPass>(executionOrder, renderPass);
        auto&& iter = std::lower_bound(renderPasses.begin(), renderPasses.end(), pair, [](const std::pair<int, RenderPass>& left, const std::pair<int, RenderPass>& right)
            { return left.first < right.first; });
        if (iter == renderPasses.end())
        {
            renderPasses.emplace_back(pair);
            iter = renderPasses.end() - 1;
        }
        else
            iter = renderPasses.insert(iter, pair);

        return iter->second;
    }

    std::size_t Graphics::Window::findRenderPass(int executionOrder) const
    {
        auto lower = renderPasses.begin();
        auto upper = renderPasses.end() - 1;
        while (lower <= upper)
        {
            auto mid = lower + (upper - lower) / 2;
            if (executionOrder == mid->first)
                return std::distance(renderPasses.begin(), mid);
            else if (executionOrder < mid->first)
                upper = mid - 1;
            else
                lower = mid + 1;
        }

        assert(!"renderpass was not found!");
        return std::numeric_limits<std::size_t>::max();
    }


}  // namespace mall