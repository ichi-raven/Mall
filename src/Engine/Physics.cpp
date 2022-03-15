#include "../../include/Engine/Physics.hpp"

namespace mall
{
    Physics::Physics()
        : mCollisionObjectID(1)
    {
        mCollisionConfiguration = std::unique_ptr<btDefaultCollisionConfiguration>(new btDefaultCollisionConfiguration());

        /// use the default collision dispatcher. For parallel processing you can use
        /// a diffent dispatcher (see Extras/BulletMultiThreaded)
        mDispatcher = std::unique_ptr<btCollisionDispatcher>(new btCollisionDispatcher(mCollisionConfiguration.get()));

        /// btDbvtBroadphase is a good general purpose broadphase. You can also try
        /// out btAxis3Sweep.
        mOverlappingPairCache = std::unique_ptr<btBroadphaseInterface>(new btDbvtBroadphase());

        /// the default constraint solver. For parallel processing you can use a
        /// different solver (see Extras/BulletMultiThreaded)
        mSolver = std::unique_ptr<btSequentialImpulseConstraintSolver>(new btSequentialImpulseConstraintSolver());

        mDynamicsWorld = std::unique_ptr<btDiscreteDynamicsWorld>(new btDiscreteDynamicsWorld(mDispatcher.get(), mOverlappingPairCache.get(), mSolver.get(), mCollisionConfiguration.get()));

        mDynamicsWorld->setGravity(btVector3(0, 10, 0));
    }

    void Physics::changeGravity(const float x, const float y, const float z)
    {
        mDynamicsWorld->setGravity(btVector3(x, y, z));
    }

    void Physics::createBox(const glm::vec3& size, const glm::vec3& origin, float mass, RigidBodyData& rigidBodyData)
    {
        RigidBody rigidBody;

        rigidBody.pCollisionShape =
            std::unique_ptr<btCollisionShape>(new btBoxShape(btVector3(size.x, size.y, size.z)));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(origin.x, origin.y, origin.z));

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            rigidBody.pCollisionShape->calculateLocalInertia(mass, localInertia);

        // using motionstate is optional, it provides interpolation capabilities,
        // and only synchronizes 'active' objects
        rigidBody.pMotionState =
            std::unique_ptr<btDefaultMotionState>(new btDefaultMotionState(transform));
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, rigidBody.pMotionState.get(), rigidBody.pCollisionShape.get(), localInertia);
        rigidBody.pRigidBody = std::unique_ptr<btRigidBody>(new btRigidBody(rbInfo));

        // add the body to the dynamics world
        auto&& rb = mRigidBodies.emplace(mCollisionObjectID++, std::move(rigidBody)).first->second;

        mDynamicsWorld->addRigidBody(rb.pRigidBody.get());
        rigidBodyData.rigidBody.create(rb.pRigidBody.get());
        rigidBodyData.objectID = mCollisionObjectID;
    }

    void Physics::destroy(RigidBodyData& rigidBodyData)
    {
        auto&& rigidBody = mRigidBodies.at(rigidBodyData.objectID);

        mDynamicsWorld->removeCollisionObject(rigidBody.pRigidBody.get());

        rigidBody.pCollisionShape.reset();
        rigidBody.pMotionState.reset();
        rigidBody.pRigidBody.reset();

        mRigidBodies.erase(rigidBodyData.objectID);

        rigidBodyData.objectID = 0;
    }

    void Physics::destroyAll()
    {
        for (auto& pair : mRigidBodies)
        {
            mDynamicsWorld->removeCollisionObject(pair.second.pRigidBody.get());

            pair.second.pCollisionShape.reset();
            pair.second.pMotionState.reset();
            pair.second.pRigidBody.reset();
        }
    }

    void Physics::update(const float deltaTime)
    {
        mDynamicsWorld->stepSimulation(deltaTime, 10);
    }

}  // namespace mall