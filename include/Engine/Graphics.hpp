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
            eDeferred = 1,
            eLighting,
            eForward,
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

        uint32_t createWindow(const uint32_t width, const uint32_t height, const char* windowName, bool fullScreen = false, const uint32_t frameCount = 3, const bool vsync = false);
        uint32_t createWindow(const Cutlass::WindowInfo& wi);

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

        Cutlass::HGraphicsPipeline createGraphicsPipeline(
            const Cutlass::GraphicsPipelineInfo& gpi,
            const uint32_t windowID = 0);

        const GBuffer& getGBuffer(const uint32_t windowID = 0) const;

        Cutlass::HRenderPass getRenderPass(const DefaultRenderPass passID, const uint32_t windowID = 0) const;

        Cutlass::HRenderPass getRenderPass(const uint32_t additionalPassID, const uint32_t windowID = 0) const;

        uint32_t addRenderPass(const Cutlass::RenderPassInfo& rpi, uint32_t executionOrder, const uint32_t windowID = 0);

        void createSubCommand(const DefaultRenderPass passID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);
        void createSubCommand(const uint32_t additionalPassID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);

        void destroySubCommand(const Cutlass::HCommandBuffer& cb);

        void writeCommand(const DefaultRenderPass passID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);
        void writeCommand(const uint32_t additionalPassID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);

        void update();

    private:
        struct RenderPass
        {
            Cutlass::HRenderPass renderPass;
            std::vector<Cutlass::HCommandBuffer> subCommands;
            Cutlass::HCommandBuffer command;
        };

        struct Window
        {
            Window()
                : width(0)
                , height(0)
                , nextPassID(0)
            {
            }

            uint32_t width;
            uint32_t height;

            Cutlass::HWindow window;
            GBuffer gBuffer;
            Cutlass::HTexture finalRT;
            Cutlass::HTexture depthBuffer;

            RenderPass deferredPass;
            RenderPass lightingPass;
            RenderPass forwardPass;
            std::map<uint32_t, RenderPass> additionalPasses;
            uint32_t nextPassID;

            Cutlass::HRenderPass presentPass;
            Cutlass::HGraphicsPipeline presentPipeline;
            Cutlass::CommandList presentCommandList;
            Cutlass::HCommandBuffer presentCommandBuffer;
        };

        uint32_t mMaxWidth;
        uint32_t mMaxHeight;

        std::shared_ptr<Cutlass::Context> mpContext;

        std::vector<Window> mWindows;
    };
}  // namespace mall

#endif