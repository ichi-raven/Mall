#include "../../include/System/SampleSystem.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <string>

#include "../../include/ComponentData/CameraData.hpp"
#include "../../include/ComponentData/LightData.hpp"
#include "../../include/ComponentData/MaterialData.hpp"
#include "../../include/ComponentData/MeshData.hpp"
#include "../../include/ComponentData/SoundData.hpp"
#include "../../include/ComponentData/SpriteData.hpp"
#include "../../include/ComponentData/TransformData.hpp"

void SampleSystem::onInit()
{
    mExecutionOrder = 0;

    mTestModel  = createEntity<mall::SkeletalMeshData, mall::MaterialData, mall::TransformData>();
    mTestCamera = createEntity<mall::CameraData, mall::TransformData>();
    mTestLight  = createEntity<mall::LightData, mall::TransformData>();

    mTestSprite = createEntity<mall::TextData, mall::TransformData>();

    auto sound = createEntity<mall::SoundData>();

    auto& resourceBank = common().resourceBank;

    resourceBank->load(
        "resources/models/CesiumMan/glTF/CesiumMan.gltf",
        //"resources/models/utakover1.01/utako2.pmx",
        getComponentData<mall::SkeletalMeshData>(*mTestModel),
        getComponentData<mall::MaterialData>(*mTestModel));

    auto& textData = getComponentData<mall::TextData>(*mTestSprite);
    resourceBank->load("resources/fonts/NikkyouSans-mLKax.ttf", textData);
    textData.centerFlag = true;
    textData.setText(L"日本語test", 600, 256, glm::vec4(1.f, 0, 0, 1.f));

    auto& spriteTrans = getComponentData<mall::TransformData>(*mTestSprite);
    spriteTrans.setup(glm::vec3(500, 500, 0));

    auto& soundData = getComponentData<mall::SoundData>(sound);
    bool res        = resourceBank->load("resources/sounds/th06_14.wav", soundData);
    assert(res);
    soundData.volumeRate = 0.25f;
    common().audio->play(soundData);

    auto&& camera = getComponentData<mall::CameraData>(*mTestCamera);
    uint32_t width, height;
    common().graphics->getWindowSize(width, height);
    camera.setup(glm::vec3(0, 0, -10), glm::vec3(0, 1, 0), 1.f * width / height);

    auto&& lightData = getComponentData<mall::LightData>(*mTestLight);
    lightData.setupDirectionalLight(glm::vec4(0.f, 0.1f, 0.1f, 1.f), glm::vec3(0.f, -30.f, -2.f));

    auto&& lightTrans = getComponentData<mall::TransformData>(*mTestLight);
    lightTrans.setup();
    lightTrans.pos = glm::vec3(0.f, 0.1f, -3.f);

    auto&& cameraTrans = getComponentData<mall::TransformData>(*mTestCamera);
    cameraTrans.setup();
    cameraTrans.rot = glm::quat(glm::vec3(0));

    auto&& modelTrans = getComponentData<mall::TransformData>(*mTestModel);
    modelTrans.setup(glm::vec3(0.f, 1.f, -3.f));
    modelTrans.rot = glm::quat(glm::vec3(0, 0, glm::radians(180.f)));
    // modelTrans.scale = glm::vec3(0.12f);
}

void SampleSystem::onUpdate()
{
    static std::uint32_t frame = 0;
    if (++frame % 30 == 0)
    {
        auto& textData = getComponentData<mall::TextData>(*mTestSprite);
        // common().resourceBank->load("resources/fonts/NikkyouSans-mLKax.ttf", textData);
        textData.centerFlag = true;
        textData.setText(std::to_wstring(static_cast<int>(1. / common().deltaTime)), 600, 256, glm::vec4(1.f, 0, 0, 1.f));
        frame = 0;
    }

    auto& trans = getComponentData<mall::TransformData>(*mTestSprite);
    trans.vel   = glm::vec3(0);

    float speed = 200.f;

    if (common().input->getKey(Cutlass::Key::Up))
    {
        trans.vel.z = speed;
    }
    if (common().input->getKey(Cutlass::Key::Down))
    {
        trans.vel.z = -speed;
    }

    if (common().input->getKey(Cutlass::Key::A))
    {
        trans.vel.x = -speed;
    }
    if (common().input->getKey(Cutlass::Key::D))
    {
        trans.vel.x = speed;
    }

    if (common().input->getKey(Cutlass::Key::W))
    {
        trans.vel.y = -speed;
    }
    if (common().input->getKey(Cutlass::Key::S))
    {
        trans.vel.y = speed;
    }

    // trans.rot = glm::quat(static_cast<float>(common().deltaTime) * glm::vec3(0, 0, glm::radians(90.f))) * trans.rot;

    if (common().input->getKey(Cutlass::Key::Escape))
    {
        endAll();
    }
}

void SampleSystem::onEnd()
{
}