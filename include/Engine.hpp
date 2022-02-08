#ifndef MALL_GLOBAL_HPP_
#define MALL_GLOBAL_HPP_

#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/ResourceBank.hpp"

#include <memory>

/**
 * @brief Engine提供機能
 * @warning 他の場所に記述することを禁止する
 * @warning 基本、直接値を書き換える目的のオブジェクトは置かない
 */
namespace mall
{
    struct Engine
    {
        std::unique_ptr<Graphics> graphics;
        std::unique_ptr<Input> input;
        std::unique_ptr<ResourceBank> resourceBank;
    };
}  // namespace mall

#endif