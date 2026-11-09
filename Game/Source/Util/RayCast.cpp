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
                /** 当たらないようにするオブジェクト */
                const btCollisionObject* ignoreObject = nullptr;
                /** キャラクターには当たらないか */
                bool isIgnoreCharacter = false;


                ClosestRayCallback(const btVector3& from, const btVector3& to)
                    : btCollisionWorld::ClosestRayResultCallback(from, to)
                {}


                bool needsCollision(btBroadphaseProxy* proxy) const override
                {
                    const btCollisionObject* object = static_cast<const btCollisionObject*>(proxy->m_clientObject);
                    // 自分の体には当たらない
                    if (object == ignoreObject) return false;
                    // キャラクターの体には当たらない
                    if (isIgnoreCharacter && object->getUserIndex() == EnCollisionAttr::enCollisionAttr_Character) return false;
                    
                    return btCollisionWorld::ClosestRayResultCallback::needsCollision(proxy);
                }
            };
        }



        RayHit RayCastClosest(
            const Vector3& start, 
            const Vector3& end, 
            const btCollisionObject* ignoreObject,
            const RayFilter filter
        )
        {
            const btVector3 from(start.x, start.y, start.z);
            const btVector3 to(end.x, end.y, end.z);

            ClosestRayCallback callback(from, to);
            callback.ignoreObject = ignoreObject;
            callback.isIgnoreCharacter = (filter == RayFilter::IgnoreCharacter);
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