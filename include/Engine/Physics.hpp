#ifndef MALL_ENGINE_PHYSICS_HPP_
#define MALL_ENGINE_PHYSICS_HPP_

#include "bullet/btBulletDynamicsCommon.h"

#include <memory>

#include "../ComponentData/RigidBodyData.hpp"

namespace mall
{
    class Physics
    {
    public:
        Physics();

        void changeGravity(const float x, const float y, const float z);

        void createBox(const glm::vec3& size, const glm::vec3& origin, float mass, RigidBodyData& rigidBody);

        void createSphere();

        void createCapsule();

        void createCylinder();

        void createCone();

        void destroy(RigidBodyData& rigidBody);

        void destroyAll();

        void update(const float deltaTime);

    private:

        void createCollisionObject(const std::unique_ptr<btCollisionShape>& shape, const btTransform& transform, const float mass);

        struct RigidBody
        {
            std::unique_ptr<btCollisionShape> pCollisionShape;
            std::unique_ptr<btDefaultMotionState> pMotionState;
            std::unique_ptr<btRigidBody> pRigidBody;
        };

        /// collision configuration contains default setup for memory, collision
        /// setup. Advanced users can create their own configuration.
        std::unique_ptr<btDefaultCollisionConfiguration> mCollisionConfiguration;

        /// use the default collision dispatcher. For parallel processing you can use
        /// a diffent dispatcher (see Extras/BulletMultiThreaded)
        std::unique_ptr<btCollisionDispatcher> mDispatcher;

        /// btDbvtBroadphase is a good general purpose broadphase. You can also try
        /// out btAxis3Sweep.
        std::unique_ptr<btBroadphaseInterface> mOverlappingPairCache;

        /// the default constraint solver. For parallel processing you can use a
        /// different solver (see Extras/BulletMultiThreaded)
        std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;

        std::unique_ptr<btDiscreteDynamicsWorld> mDynamicsWorld;
    
        int mCollisionObjectID;
        std::unordered_map<int, RigidBody> mRigidBodies;
    };
}  // namespace mall

#endif