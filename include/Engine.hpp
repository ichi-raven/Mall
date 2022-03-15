#ifndef MALL_GLOBAL_HPP_
#define MALL_GLOBAL_HPP_

#include <memory>

#include "Engine/Audio.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/Physics.hpp"
#include "Engine/ResourceBank.hpp"

/**
 * @brief Engine提供機能
 * @warning 他の場所に記述することを禁止する
 * @warning 基本、直接値を書き換える目的のオブジェクトは置かない
 */
namespace mall
{
    struct Engine
    {
        std::unique_ptr<Audio> audio;
        std::unique_ptr<Graphics> graphics;
        std::unique_ptr<Input> input;
        std::unique_ptr<Physics> physics;
        std::unique_ptr<ResourceBank> resourceBank;

        double deltaTime;
    };
}  // namespace mall

#endif