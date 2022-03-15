#ifndef MALL_COMPONENTDATA_RIGITBODYDATA_HPP_
#define MALL_COMPONENTDATA_RIGITBODYDATA_HPP_

#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>

#include "bullet/btBulletDynamicsCommon.h"

#include "../Utility.hpp"

namespace mall
{
    struct RigidBodyData : public mvecs::IComponentData
    {
        COMPONENT_DATA(RigidBodyData)

        TUPointer<btRigidBody> rigidBody;

        int objectID;

        glm::mat4 world;
    };
}  // namespace mall

#endif