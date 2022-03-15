#ifndef MALL_COMPONENTDATA_TEXTDATA_HPP_
#define MALL_COMPONENTDATA_TEXTDATA_HPP_

#include <stb/stb_truetype.h>

#include <Cutlass/Texture.hpp>
#include <MVECS/IComponentData.hpp>
#include <memory>
#include <string_view>

#include <glm/glm.hpp>

#include "../Utility.hpp"

namespace mall
{
    struct TextData : public mvecs::IComponentData
    {
        COMPONENT_DATA(TextData)

        struct RenderingInfo
        {
            struct Scene2DCBParam
            {
                glm::mat4 proj;
            };

            struct Vertex
            {
                glm::vec3 pos;
                glm::vec2 uv;
            };

            Cutlass::HBuffer spriteVB;
        };

        struct RGBA
        {
            RGBA()
                : r(0)
                , g(0)
                , b(0)
                , a(255)
            {
            }

            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        void setText(std::wstring_view wstr, std::uint32_t width, std::uint32_t height, glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f));

        TUArray<const wchar_t> string;
        TUPointer<stbtt_fontinfo> fontInfo;
        TUPointer<std::unique_ptr<unsigned char>> fontBuffer;
        glm::vec4 color;
        bool centerFlag;
        std::uint32_t width;
        std::uint32_t height;

        Cutlass::HTexture texture;

        RenderingInfo renderingInfo;
    };
}  // namespace mall

#endif