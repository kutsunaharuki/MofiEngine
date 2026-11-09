/**
 * @file Vision.cpp
 * @brief 汎用的なビジョンクラス
 */
#include "stdafx.h"
#include "Vision.h"
#include "Source/Util/RayCast.h"


namespace app
{
    namespace ai
    {
        Vision::Vision()
            : m_setting()
            , m_isSeeing(false)
            , m_seeTimer(0.0f)
        {}
        
        
        Vision::~Vision()
        {}
        
        
        bool Vision::Update(
            const float deltaTime, 
            const Vector3& eyePos, 
            const Vector3& forward, 
            const Vector3& targetPos,
            const btCollisionObject* selfObject,
            const btCollisionObject* targetObject
        )
        {
            m_isSeeing = CanSee(eyePos, forward, targetPos,selfObject,targetObject);

            // 見えていない間は 0 に戻す(ちらっと見えただけでは気づかない)
            if (!m_isSeeing)
            {
                m_seeTimer = 0.0f;
                return false;
            }

            // 気づく時間を超えた最初のフレームだけ true
            const bool wasNoticed = m_seeTimer >= m_setting.noticeTime;
            m_seeTimer += deltaTime;
            return !wasNoticed && m_seeTimer >= m_setting.noticeTime;
        }


        bool Vision::CanSee(
            const Vector3& eyePos,
            const Vector3& forward,
            const Vector3& targetPos,
            const btCollisionObject* selfObject,
            const btCollisionObject* targetObject
        ) const
        {
            Vector3 toTarget = targetPos - eyePos;
            const float distance = toTarget.Length();

            /** 距離 */
            // 距離が遠すぎると見えない扱いに
            if (distance <= 0.0f || distance > m_setting.viewDistance) return false;

            /** 角度 */
            // 前の向きと内積が cos(視野の半分) より小さければ、視野の外
            Vector3 direction = toTarget;
            direction.Normalize();
            const float halfAngleRad = Math::DegToRad(m_setting.viewAngle * 0.5f);
            if (forward.Dot(direction) < cosf(halfAngleRad)) return false;

            /** レイ */
            // 一番近くで当たった物が相手の体なら、間に何もない
            const util::RayHit hit = util::RayCastClosest(eyePos, targetPos, selfObject);
            
            return hit.isHit && hit.hitObject == targetObject;
        }
    }
}
