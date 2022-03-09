#include "../../include/ComponentData/TextData.hpp"

namespace mall
{
    void TextData::setText(std::wstring_view text, std::uint32_t width_, std::uint32_t height_, glm::vec4 color)
    {
        static std::wstring wstr;
        wstr = text;
        width = width_;
        height = height_;
        string.create(wstr.data(), wstr.size());
    }
}