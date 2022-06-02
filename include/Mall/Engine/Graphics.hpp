#ifndef MALL_GRAPHICS_HPP_
#define MALL_GRAPHICS_HPP_

#include <Cutlass/Context.hpp>
#include <functional>
#include <limits>
#include <map>

namespace mall
{
    class Graphics
    {
    public:
        enum class DefaultRenderPass
        {
            eGeometry = 1,
            eLighting,
            eForward,
            eSprite,
        };

        struct GBuffer
        {
            Cutlass::HTexture albedo;
            Cutlass::HTexture normal;
            Cutlass::HTexture worldPos;
            Cutlass::HTexture metalic;
            Cutlass::HTexture roughness;
        };

        Graphics(const std::shared_ptr<Cutlass::Context>& context);

        Graphics(const std::shared_ptr<Cutlass::Context>& context, const std::vector<Cutlass::WindowInfo>& windows);

        ~Graphics();

        uint32_t createWindow(const uint32_t width, const uint32_t height, const char* windowName, bool fullScreen = false, const uint32_t frameCount = 3, const bool vsync = false);
        uint32_t createWindow(const Cutlass::WindowInfo& wi);

        void getWindowSize(uint32_t& width_out, uint32_t& height_out, uint32_t windowID = 0);

        //バッファ作成・破棄
        Cutlass::HBuffer createBuffer(const Cutlass::BufferInfo& info);
        void destroyBuffer(const Cutlass::HBuffer& handle);

        //バッファ書き込み
        void writeBuffer(const size_t size, const void* const pData, const Cutlass::HBuffer& handle);

        //テクスチャ作成・破棄
        Cutlass::HTexture createTexture(const Cutlass::TextureInfo& info);
        void destroyTexture(const Cutlass::HTexture& handle);

        //ファイルからテクスチャ作成
        Cutlass::HTexture createTextureFromFile(const char* fileName);

        //テクスチャからサイズを取得する
        void getTextureSize(const Cutlass::HTexture& handle, uint32_t& width_out, uint32_t& height_out, uint32_t& depth_out);

        //テクスチャにデータ書き込み(使用注意, 書き込むデータのサイズはテクスチャのサイズに従うもの以外危険)
        void writeTexture(const void* const pData, const Cutlass::HTexture& handle);

        Cutlass::HGraphicsPipeline getGraphicsPipeline(
            const Cutlass::GraphicsPipelineInfo& gpi,
            const uint32_t windowID = 0);

        const GBuffer& getGBuffer(const uint32_t windowID = 0) const;

        Cutlass::HRenderPass getRenderPass(const DefaultRenderPass passID, const uint32_t windowID = 0) const;

        Cutlass::HRenderPass getPrepass(const uint32_t additionalPassID, const uint32_t windowID = 0) const;

        Cutlass::HRenderPass getPostpass(const uint32_t additionalPassID, const uint32_t windowID = 0) const;

        uint32_t addPrepass(const Cutlass::RenderPassInfo& rpi, uint32_t executionOrder, const uint32_t windowID = 0);

        uint32_t addPostpass(const Cutlass::RenderPassInfo& rpi, uint32_t executionOrder, const uint32_t windowID = 0);

        Cutlass::HCommandBuffer createSubCommand(const DefaultRenderPass passID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);
        Cutlass::HCommandBuffer createSubCommandPrepass(const uint32_t additionalPassID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);
        Cutlass::HCommandBuffer createSubCommandPostpass(const uint32_t additionalPassID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);

        void destroySubCommand(const Cutlass::HCommandBuffer& cb);

        void writeCommand(const DefaultRenderPass passID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);
        void writeCommandPrepass(const uint32_t prePassID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);
        void writeCommandPostpass(const uint32_t postPassID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);

        void update();

        bool shouldClose();

    private:
        struct RenderPass
        {
            Cutlass::HRenderPass renderPass;
            Cutlass::HCommandBuffer command;
        };

        struct Window
        {
            Window()
                : width(0)
                , height(0)
                , frameCount(3)
            {
            }

            uint32_t width;
            uint32_t height;
            uint32_t frameCount;

            Cutlass::HWindow window;
            GBuffer gBuffer;
            Cutlass::HTexture finalRT;
            Cutlass::HTexture depthBuffer;

            std::map<uint32_t, RenderPass> prePasses;
            RenderPass geometryPass;
            RenderPass lightingPass;
            RenderPass forwardPass;
            RenderPass spritePass;
            std::map<uint32_t, RenderPass> postPasses;
            //uint32_t nextPassID;
            std::unordered_map<Cutlass::GraphicsPipelineInfo, Cutlass::HGraphicsPipeline> graphicsPipelines;

            Cutlass::HRenderPass presentPass;
            Cutlass::HGraphicsPipeline presentPipeline;
            std::vector<Cutlass::CommandList> presentCommandLists;
            Cutlass::HCommandBuffer presentCommandBuffer;
        };

        uint32_t mMaxWidth;
        uint32_t mMaxHeight;

        std::shared_ptr<Cutlass::Context> mpContext;

        std::vector<Window> mWindows;

        Cutlass::HTexture mDebugTex;
    };
}  // namespace mall

#endif