#include "../../include/Mall/ComponentData/TextData.hpp"

namespace mall
{
    void TextData::setText(std::wstring_view wstr, std::uint32_t width_, std::uint32_t height_, glm::vec4 color_, bool centerFlag_)
    {
        static std::wstring ws;
        ws = wstr;
        width = width_;
        height = height_;
        color = color_;
        centerFlag = centerFlag_;
        
        string.create(ws.data(), ws.size());
    }
}