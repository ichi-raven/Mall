#ifndef MALL_SYSTEM_TEXTSYSTEM_HPP_
#define MALL_SYSTEM_TEXTSYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include <chrono>

#include "../ComponentData/TextData.hpp"
#include "../Engine.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class TextSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(TextSystem, Key, Common)

    public:
        virtual void onInit()
        {

        }

        virtual void onUpdate()
        {
            this->template forEach<mall::TextData>(
                [&](mall::TextData& text)
                {
                    if (text.string.size() == 0)
                        return;

                    /* create a bitmap */
                    const uint32_t bitmap_w = text.width;  /* Width of bitmap */
                    const uint32_t bitmap_h = text.height; /* Height of bitmap */
                    unsigned char* bitmap   = (unsigned char*)calloc(bitmap_w * bitmap_h, sizeof(unsigned char));

                    /* "STB"unicode encoding of */
                    // char word[20] = "test image";
                    // std::string str = "test string";

                    /* Calculate font scaling */
                    float pixels = 64.0;                                                    /* Font size (font size) */
                    float scale  = stbtt_ScaleForPixelHeight(text.fontInfo.data(), pixels); /* scale = pixels / (ascent - descent) */

                    /**
                     * Get the measurement in the vertical direction
                     * ascent: The height of the font from the baseline to the top;
                     * descent: The height from baseline to bottom is usually negative;
                     * lineGap: The distance between two fonts;
                     * The line spacing is: ascent - descent + lineGap.
                     */
                    int ascent  = 0;
                    int descent = 0;
                    int lineGap = 0;
                    stbtt_GetFontVMetrics(text.fontInfo.data(), &ascent, &descent, &lineGap);

                    /* Adjust word height according to zoom */
                    ascent  = roundf(ascent * scale);
                    descent = roundf(descent * scale);

                    int x = 0; /*x of bitmap*/

                    /* Cyclic loading of each character in word */
                    // for (int i = 0; i < strlen(word); ++i)
                    for (std::size_t i = 0; i < text.string.size(); ++i)
                    {
                        /**
                         * Get the measurement in the horizontal direction
                         * advanceWidth: Word width;
                         * leftSideBearing: Left side position;
                         */
                        int advanceWidth    = 0;
                        int leftSideBearing = 0;
                        stbtt_GetCodepointHMetrics(text.fontInfo.data(), text.string[i], &advanceWidth, &leftSideBearing);

                        /* Gets the border of a character */
                        int c_x1, c_y1, c_x2, c_y2;
                        stbtt_GetCodepointBitmapBox(text.fontInfo.data(), text.string[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

                        /* Calculate the y of the bitmap (different characters have different heights) */
                        int y = ascent + c_y1;

                        /* Render character */
                        int byteOffset = x + roundf(leftSideBearing * scale) + (y * bitmap_w);
                        stbtt_MakeCodepointBitmap(text.fontInfo.data(), bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmap_w, scale, scale, text.string[i]);

                        /* Adjust x */
                        x += roundf(advanceWidth * scale);

                        /* kerning */
                        if (i < text.string.size() - 1)
                        {
                            int kern;
                            kern = stbtt_GetCodepointKernAdvance(text.fontInfo.data(), text.string[i], text.string[i + 1]);
                            x += roundf(kern * scale);
                        }
                    }

                    // std::unique_ptr<RGBA[][bitmap_w]> writeData(new(std::nothrow) RGBA[bitmap_h][bitmap_w]);
                    TextData::RGBA* writeData = (TextData::RGBA*)calloc(sizeof(TextData::RGBA), bitmap_w * bitmap_h);

                    {
                        bool alphaTest   = false;
                        size_t idx       = 0;
                        auto normalColor = text.color * 255.f;
                        for (size_t r = 0; r < bitmap_h; ++r)
                            for (size_t c = 0; c < bitmap_w; ++c)
                            {
                                idx       = r * bitmap_w + c;
                                alphaTest = bitmap[idx] > 150;
                                if (alphaTest)
                                {
                                    writeData[idx].r = normalColor.r;
                                    writeData[idx].g = normalColor.g;
                                    writeData[idx].b = normalColor.b;
                                    writeData[idx].a = 255;
                                }
                                else
                                    writeData[idx].a = 0;
                            }
                    }

                    auto& graphics = this->common().graphics;

                    {
                        uint32_t w, h, d;
                        graphics->getTextureSize(text.texture, w, h, d);
                        assert(d == 1);

                        if (w != bitmap_w || h != bitmap_h)
                        {
                            Cutlass::TextureInfo ti;
                            ti.setSRTex2D(bitmap_w, bitmap_h, true);

                            Cutlass::HTexture handle = graphics->createTexture(ti);
                            graphics->writeTexture(writeData, handle);
                            graphics->destroyTexture(text.texture);
                            text.texture = handle;
                        }
                        else
                            graphics->writeTexture(writeData, text.texture);
                    }

                    free(bitmap);
                    free(writeData);
                });
        }

        virtual void onEnd()
        {
        }

    protected:
    };
}  // namespace mall

#endif