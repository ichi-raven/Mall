#ifndef MALL_ENGINE_HPP_
#define MALL_ENGINE_HPP_

#include <MVECS/Application.hpp>
#include <MVECS/ISystem.hpp>
#include <MVECS/World.hpp>
#include <chrono>
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
        std::uint32_t frame;
    };

    template <typename Key, typename Common, typename = std::enable_if_t<std::is_base_of_v<Engine, Common>>>
    void initialize(const char* appName, Cutlass::WindowInfo& defaultWindow, mvecs::Application<Key, Common>& app)
    {
        auto&& pContext = std::make_shared<Cutlass::Context>();

#ifdef _DEBUG
        pContext->initialize(appName, true);
#else
        pContext->initialize(appName, false);
#endif

        app.common().audio        = std::make_unique<Audio>();
        app.common().graphics     = std::make_unique<Graphics>(pContext);
        app.common().input        = std::make_unique<Input>(pContext);
        app.common().physics      = std::make_unique<Physics>();
        app.common().resourceBank = std::make_unique<ResourceBank>(pContext);

        app.common().graphics->createWindow(defaultWindow);
        app.common().frame = 0;
    }

    template <typename Key, typename Common, typename = std::enable_if_t<std::is_base_of_v<Engine, Common>>>
    void update(mvecs::Application<Key, Common>& app)
    {
        static std::chrono::high_resolution_clock::time_point now, prev = std::chrono::high_resolution_clock::now();

        {  //フレーム数, fps
            now                    = std::chrono::high_resolution_clock::now();
            app.common().deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - prev).count() / 1000000.;
        }

        app.common().input->update();

        app.update();

        app.common().physics->update(app.common().deltaTime);
        app.common().graphics->update();

        {  //フレーム, 時刻更新
            ++app.common().frame;
            prev = now;
        }
    }

}  // namespace mall

#endif