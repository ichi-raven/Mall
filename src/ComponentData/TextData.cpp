#include "../../include/ComponentData/TextData.hpp"

namespace mall
{
    void TextData::setText(std::wstring_view text, std::uint32_t width_, std::uint32_t height_, glm::vec4 color_)
    {
        static std::wstring wstr;
        wstr = text;
        width = width_;
        height = height_;
        color = color_;
        string.create(wstr.data(), wstr.size());
    }
}