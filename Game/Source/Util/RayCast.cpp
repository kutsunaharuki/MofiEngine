/**
 * @file RayCast.cpp
 * @brief 一番近くで当たったものを返すレイ
 */
#include "stdafx.h"
#include "RayCast.h"


namespace app
{
    namespace util
    {
        namespace
        {
            /**
             * @brief 一番近い物だけを残すコールバック
             */
            struct ClosestRayCallback : public btCollisionWorld::ClosestRayResultCallback
            {
                const btCollisionObject* ignoreObject = nullptr;

                ClosestRayCallback(const btVector3& from, const btVector3& to)
                    : btCollisionWorld::ClosestRayResultCallback(from, to)
                {}


                bool needsCollision(btBroadphaseProxy* proxy) const override
                {
                    // 自分の体には当たらない
                    if (proxy->m_clientObject == ignoreObject) return false;
                    return btCollisionWorld::ClosestRayResultCallback::needsCollision(proxy);
                }
            };
        }



        RayHit RayCastClosest(const Vector3& start, const Vector3& end, const btCollisionObject* ignoreObject)
        {
            const btVector3 from(start.x, start.y, start.z);
            const btVector3 to(end.x, end.y, end.z);

            ClosestRayCallback callback(from, to);
            callback.ignoreObject = ignoreObject;
            PhysicsWorld::GetInstance()->GetDynamicWorld()->rayTest(from, to, callback);


            RayHit hit;
            hit.isHit = callback.hasHit();
            if (hit.isHit)
            {
                const btVector3& point = callback.m_hitPointWorld;
                hit.position.Set(point.x(), point.y(), point.z());
                hit.hitObject = callback.m_collisionObject;
            }

            return hit;
        }
    }
}