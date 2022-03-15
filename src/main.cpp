#include <Cutlass/Cutlass.hpp>
#include <MVECS/MVECS.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "../include/AppInfo.hpp"
#include "../include/System/EngineBasicSystem.hpp"
#include "../include/System/SampleSystem.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::enable_if_t<std::is_base_of_v<Engine, Common>>>
    void initialize(const char* appName, mvecs::Application<Key, Common>& app)
    {
        auto&& pContext = std::make_shared<Cutlass::Context>();

#ifdef _DEBUG
        pContext->initialize(appName, true);
#else
        pContext->initialize(appName, false);
#endif

        app.common().audio        = std::make_unique<Audio>();
        app.common().graphics     = std::make_unique<Graphics>(pContext);
        app.common().physics      = std::make_unique<Physics>();
        app.common().input        = std::make_unique<Input>(pContext);
        app.common().resourceBank = std::make_unique<ResourceBank>(pContext);
    }

    template <typename Key, typename Common, typename = std::enable_if_t<std::is_base_of_v<Engine, Common>>>
    float update(mvecs::Application<Key, Common>& app)
    {
        constexpr std::size_t sampleNum = 10;
        static std::array<float, sampleNum> times;  // 10f平均でfpsを計測
        static std::chrono::high_resolution_clock::time_point now, prev = std::chrono::high_resolution_clock::now();
        static std::uint32_t frame = 0;

        {  //フレーム数, fps
            now                    = std::chrono::high_resolution_clock::now();
            app.common().deltaTime = times[frame % 10] = std::chrono::duration_cast<std::chrono::microseconds>(now - prev).count() / 1000000.;
            std::cerr << "now frame : " << frame << "\n";
            std::cerr << "FPS : " << 1. / (std::accumulate(times.begin(), times.end(), 0.) / 10.) << "\n";
        }

        app.common().input->update();
        app.common().physics->update(app.common().deltaTime);
        app.common().graphics->update();
        app.update();

        {  //フレーム, 時刻更新
            ++frame;
            prev = now;
        }

        return (1. / (std::accumulate(times.begin(), times.end(), 0.) / 1. * sampleNum));
    }

}  // namespace mall

int main()
{
    constexpr const char* appName = "Mallsoftware";
    constexpr uint32_t width = 1920, height = 1080;

    mvecs::Application<WorldKey, Common> app;

    mall::initialize<WorldKey, Common>(appName, app);

    auto& titleWorld = app.add(WorldKey::eTitle);

    titleWorld.addSystems<mall::EngineBasicSystem<WorldKey, Common>, SampleSystem>();

    app.common().graphics->createWindow(width, height, appName, false, 3, false);

    app.start(WorldKey::eTitle);

    while (!app.ended())
    {
        mall::update(app);
    }

    return 0;
}