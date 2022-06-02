#ifndef MALL_COMPONENTDATA_CAMERADATA_HPP_
#define MALL_COMPONENTDATA_CAMERADATA_HPP_

#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>
#include <Cutlass/Buffer.hpp>

#pragma once
#include <../um/windows.h>
#undef near
#undef far

namespace mall
{
    struct CameraData : public mvecs::IComponentData
    {
        COMPONENT_DATA(CameraData)

        struct RenderingInfo
        {
            struct CameraCBParam
            {
                glm::vec3 cameraPos;
            };
        };

        void setup(glm::vec3 lookPos, glm::vec3 up, float aspect, float fovY = 45.f, float near = 0.1f, float far = 1000.f);

        bool enable;

        glm::vec3 lookPos;
        glm::vec3 up;
        float aspect;
        float fovY;
        float near;
        float far;
    };
}  // namespace mall

#endif