#include <Cutlass/Cutlass.hpp>
#include <MVECS/MVECS.hpp>
#include <iostream>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "../include/AppInfo.hpp"

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

        app.common().graphics     = std::make_unique<Graphics>(pContext);
        app.common().input        = std::make_unique<Input>(pContext);
        app.common().resourceBank = std::make_unique<ResourceBank>(pContext);
    }

    template <typename Key, typename Common, typename = std::enable_if_t<std::is_base_of_v<Engine, Common>>>
    void update(mvecs::Application<Key, Common>& app)
    {
        app.common().input->update();
        app.common().graphics->update();
        app.update();
    }
}  // namespace mall

int main()
{
    constexpr const char* appName = "Mallsoftware";
    constexpr uint32_t width = 800, height = 600;

    mvecs::Application<WorldKey, Common> app;

    mall::initialize<WorldKey, Common>(appName, app);

    app.common().graphics->createWindow(width, height, appName);

    while (!app.ended())
    {
        mall::update(app);
    }

    std::cout << "application end\n";

    return 0;
}