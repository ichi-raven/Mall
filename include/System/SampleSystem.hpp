#ifndef MALL_SYSTEM_SAMPLESYSTEM_HPP_
#define MALL_SYSTEM_SAMPLESYSTEM_HPP_

#include <MVECS/ISystem.hpp>
#include "../AppInfo.hpp"

class SampleSystem : public mvecs::ISystem<WorldKey, Common>
{
    SYSTEM(SampleSystem, WorldKey, Common)

public:
    virtual void onInit();

    virtual void onUpdate();

    virtual void onEnd();

private:
    std::optional<mvecs::Entity> mTestModel;
    std::optional<mvecs::Entity> mTestCamera;
    std::optional<mvecs::Entity> mTestLight;
};

#endif