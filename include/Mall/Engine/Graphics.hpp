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
        // ほんとうにどうしようもないときはここを書き換える
        enum class DefaultRenderPass
        {
            eGeometry = 128,
            eLighting = 256,
            eForward = 384,
            eSprite = 512,
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

        // PSO取得(無ければ作成される)
        Cutlass::HGraphicsPipeline getGraphicsPipeline(
            const Cutlass::GraphicsPipelineInfo& gpi,
            const uint32_t windowID = 0);

        const GBuffer& getGBuffer(const uint32_t windowID = 0) const;


        //Cutlass::HRenderPass getPrepass(const uint32_t additionalPassID, const uint32_t windowID = 0) const;

        //Cutlass::HRenderPass getPostpass(const uint32_t additionalPassID, const uint32_t windowID = 0) const;

        //uint32_t addPrepass(const Cutlass::RenderPassInfo& rpi, uint32_t executionOrder, const uint32_t windowID = 0);

        //uint32_t addPostpass(const Cutlass::RenderPassInfo& rpi, uint32_t executionOrder, const uint32_t windowID = 0);

        // 例えばLightingPassの後で行いたいならDefaultRenderPass::eLighting + 1とする
        void addRenderPass(const Cutlass::RenderPassInfo& rpi, const int executionOrder, std::string_view passName, const uint32_t windowID = 0);

        int getExecutionOrder(const DefaultRenderPass passID) const;

        Cutlass::HRenderPass getRenderPass(const DefaultRenderPass passID, const uint32_t windowID = 0) const;
        Cutlass::HRenderPass getRenderPass(const int executionOrder, const uint32_t windowID = 0) const;

        std::pair<int, Cutlass::HRenderPass> findRenderPass(std::string_view passName, const uint32_t windowID = 0) const;

        // TODO:
        //Cutlass::HCommandBuffer createSubCommand(const DefaultRenderPass passID, const Cutlass::SubCommandList& cl, const uint32_t windowID = 0);
        //void destroySubCommand(const Cutlass::HCommandBuffer& cb);

        void writeCommand(const DefaultRenderPass passID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);
        void writeCommand(const int executionOrder, const Cutlass::CommandList& cl, const uint32_t windowID = 0);

        //void writeCommandPrepass(const uint32_t prePassID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);
        //void writeCommandPostpass(const uint32_t postPassID, const Cutlass::CommandList& cl, const uint32_t windowID = 0);

        void update();

        bool shouldClose();

    private:
        struct RenderPass
        {
            Cutlass::HRenderPass renderPass;
            Cutlass::HCommandBuffer command;
            std::string passName;
        };

        struct Window
        {
            Window()
                : width(0)
                , height(0)
                , frameCount(3)
                , geometryPassIndex(0)
                , lightingPassIndex(1)
                , forwardPassIndex(2)
                , spritePassIndex(3)
            {
            }

            uint32_t width;
            uint32_t height;
            uint32_t frameCount;

            Cutlass::HWindow window;
            GBuffer gBuffer;
            Cutlass::HTexture finalRT;
            Cutlass::HTexture depthBuffer;

            //std::map<uint32_t, RenderPass> prePasses;
            /*RenderPass geometryPass;
            RenderPass lightingPass;
            RenderPass forwardPass;
            RenderPass spritePass;*/
            //std::map<uint32_t, RenderPass> postPasses;

            std::vector<std::pair<int, RenderPass>> renderPasses;
            RenderPass& insertRenderPass(int executionOrder, const RenderPass& renderPass);

            std::size_t findRenderPass(int executionOrder) const;

            std::size_t geometryPassIndex;
            std::size_t lightingPassIndex;
            std::size_t forwardPassIndex;
            std::size_t spritePassIndex;


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