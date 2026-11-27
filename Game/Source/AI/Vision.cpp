/**
 * @file Vision.cpp
 * @brief 汎用的なビジョンクラス
 */
#include "stdafx.h"
#include "Vision.h"
#include "Source/Util/RayCast.h"
#include <cmath>


namespace app
{
    namespace ai
    {
        namespace
        {
            /** 相手の体の、線を引く高さ(targetHeight を 1 とした割合。頭・胸・ひざ) */
            constexpr float BODY_POINT_RATE_LIST[] = { 1.6f, 1.0f, 0.4f };
            static_assert(sizeof(BODY_POINT_RATE_LIST) / sizeof(BODY_POINT_RATE_LIST[0]) == Vision::BODY_POINT_COUNT,
                "BODY_POINT_RATE_LIST と BODY_POINT_COUNT の数が合っていません。");
        }


        Vision::Vision()
            : m_setting()
            , m_isSeeing(false)
            , m_seeTimer(0.0f)
            , m_strength(0.0f)
            , m_debugRayCount(0)
        {}
        
        
        Vision::~Vision()
        {}
        
        
        bool Vision::Update(
            const float deltaTime,
            const Vector3& eyePos,
            const Vector3& forward,
            const Vector3& targetFootPos,
            const float targetHeight,
            const btCollisionObject* selfObject,
            const btCollisionObject* targetObject,
            const float noticeScale
        )
        {
            m_strength = CalcStrength(eyePos, forward, targetFootPos, targetHeight, selfObject, targetObject, noticeScale);
            m_isSeeing = m_strength > 0.0f;

            // 見えていない間は 0 に戻す(ちらっと見えただけでは気づかない)
            if (!m_isSeeing)
            {
                m_seeTimer = 0.0f;
                return false;
            }

            // 気づく時間を超えた最初のフレームだけ true
            // 強さが弱いほど、見えている時間はゆっくり増える(横・遠く・体が少ししか見えない時は、気づくのが遅い)
            const bool wasNoticed = m_seeTimer >= m_setting.noticeTime;
            m_seeTimer += deltaTime * noticeScale * m_strength;
            return !wasNoticed && m_seeTimer >= m_setting.noticeTime;
        }


        float Vision::CalcStrength(
            const Vector3& eyePos,
            const Vector3& forward,
            const Vector3& targetFootPos,
            const float targetHeight,
            const btCollisionObject* selfObject,
            const btCollisionObject* targetObject,
            const float noticeScale
        ) const
        {
            // 先にマスを見る(円の外・真後ろなら、線は引かない)
            m_debugRayCount = 0;
            const float cellRate = CalcCellRate(eyePos, forward, targetFootPos, noticeScale);
            if (cellRate <= 0.0f) return 0.0f;

            const float visibleRate = CalcVisibleRate(eyePos, targetFootPos, targetHeight, selfObject, targetObject);
            return cellRate * visibleRate;
        }


        float Vision::CalcCellRate(const Vector3& eyePos, const Vector3& forward, const Vector3& targetFootPos, const float noticeScale) const
        {
            // 上から見た(XZ 平面の)向きと距離で決める
            Vector3 toTarget = targetFootPos - eyePos;
            toTarget.y = 0.0f;
            const float distance = toTarget.Length();

            /** 距離(円) */
            // 目立つほど遠くまで見える
            const float viewDistance = m_setting.viewDistance * noticeScale;
            if (distance > viewDistance || viewDistance <= 0.0f) return 0.0f;

            /** 角度(正面・横・真後ろ) */
            float angleRate = 1.0f;
            if (distance > 0.0f)
            {
                Vector3 forwardXZ = forward;
                forwardXZ.y = 0.0f;
                forwardXZ.Normalize();
                Vector3 direction = toTarget;
                direction.Normalize();

                // 正面から何度ずれているか(0 = 正面、180 = 真後ろ)
                const float dot = forwardXZ.Dot(direction);
                const float clampedDot = dot > 1.0f ? 1.0f : (dot < -1.0f ? -1.0f : dot);
                const float angleDeg = Math::RadToDeg(std::acos(clampedDot));

                if (angleDeg > m_setting.sideAngle) return 0.0f;                    // 真後ろのまわりは見えない
                if (angleDeg > m_setting.frontAngle) angleRate = m_setting.sideRate; // 横は弱い
            }

            /** 距離の輪(内側ほど強い) */
            float ringRate = 1.0f;
            if (m_setting.ringCount > 1)
            {
                // 何番目の輪か(0 = 一番内側)
                int ringIndex = static_cast<int>(distance / viewDistance * static_cast<float>(m_setting.ringCount));
                if (ringIndex >= m_setting.ringCount) ringIndex = m_setting.ringCount - 1;
                const float ringT = static_cast<float>(ringIndex) / static_cast<float>(m_setting.ringCount - 1);
                ringRate = 1.0f + (m_setting.farRate - 1.0f) * ringT;
            }

            return angleRate * ringRate;
        }


        float Vision::CalcVisibleRate(
            const Vector3& eyePos,
            const Vector3& targetFootPos,
            const float targetHeight,
            const btCollisionObject* selfObject,
            const btCollisionObject* targetObject
        ) const
        {
            // 頭・胸・ひざへ線を引いて、一番近くで当たった物が相手の体なら、その点は見えている
            // (1 点だけだと、ベンチのふちで見える・見えないが急に切りかわるので、何点か見る)
            int visibleCount = 0;
            for (const float pointRate : BODY_POINT_RATE_LIST)
            {
                Vector3 targetPos = targetFootPos;
                targetPos.y += targetHeight * pointRate;

                const util::RayHit hit = util::RayCastClosest(eyePos, targetPos, selfObject);
                const bool isVisible = hit.isHit && hit.hitObject == targetObject;
                if (isVisible) ++visibleCount;

                // デバッグで描くために覚えておく
                m_debugRayList[m_debugRayCount] = { eyePos, targetPos, isVisible };
                ++m_debugRayCount;
            }
            return static_cast<float>(visibleCount) / static_cast<float>(BODY_POINT_COUNT);
        }
    }
}
