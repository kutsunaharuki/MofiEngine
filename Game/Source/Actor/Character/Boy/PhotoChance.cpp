/**
 * @file PhotoChance.cpp
 * @brief フォトチャンス(柱からのぞいて撮れるか)を調べる
 */
#include "stdafx.h"
#include "PhotoChance.h"
#include "Object/BoyParameter.h"
#include "Source/Stage/Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include <cmath>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 視野角がとれなかった時の、画面の横の広さ(横 90 度くらい) */
			constexpr float DEFAULT_TAN_HALF_FOV_X = 1.0f;
			/** これより短い長さは 0 とみなす */
			constexpr float EPSILON = 0.0001f;


			/**
			 * @brief XZ 平面での内積
			 */
			float DotXZ(const Vector3& a, const Vector3& b)
			{
				return a.x * b.x + a.z * b.z;
			}
		}


		PhotoChance::PhotoChance()
			: m_tanHalfFovX(DEFAULT_TAN_HALF_FOV_X)
		{}


		void PhotoChance::Initialize(const stage::Stage& stage, const std::string& pillarName)
		{
			m_pillarList.clear();

			for (int i = 0; i < stage.GetObjectCount(); i++)
			{
				const StageObject* object = stage.GetStageObject(i);
				if (object == nullptr) continue;
				if (object->GetModelPath().find(pillarName) == std::string::npos) continue;

				// 置物の箱(AABB)から、XZ 平面の丸にする(横と奥の半分の長さの平均を半径にする)
				const Vector3& aabbMin = object->GetWorldAABBMin();
				const Vector3& aabbMax = object->GetWorldAABBMax();

				Pillar pillar;
				pillar.center = (aabbMin + aabbMax) * 0.5f;
				pillar.radius = ((aabbMax.x - aabbMin.x) + (aabbMax.z - aabbMin.z)) * 0.25f;
				m_pillarList.push_back(pillar);
			}

			// 画面の横の広さ(ズームしていない時の視野角で決める。ズームで左右が変わらないように)
			// ※ 視野角は縦の角度なので、横の広さは縦の広さ × 横長の比率
			const float tanHalfFovY = std::tan(g_camera3D->GetViewAngle() * 0.5f);
			const float tanHalfFovX = tanHalfFovY * g_camera3D->GetAspect();
			m_tanHalfFovX = tanHalfFovX > EPSILON ? tanHalfFovX : DEFAULT_TAN_HALF_FOV_X;
		}


		EnLeanSide PhotoChance::Update(
			const Vector3& boyPosition,
			const Vector3& forwardXZ,
			const Vector3& rightXZ,
			const std::vector<Vector3>& enemyPositionList,
			const BoyParameter& parameter)
		{
			const Pillar* pillar = FindPillarBetween(boyPosition, enemyPositionList, parameter.leanCheckDistance);
			if (pillar == nullptr) return EnLeanSide::None;

			return CalcLeanSide(*pillar, boyPosition, forwardXZ, rightXZ, parameter.leanEdgeLimit);
		}


		const PhotoChance::Pillar* PhotoChance::FindPillarBetween(const Vector3& boyPosition, const std::vector<Vector3>& enemyPositionList, const float checkDistance) const
		{
			const Pillar* nearestPillar = nullptr;
			float nearestDistance = checkDistance;

			for (const Pillar& pillar : m_pillarList)
			{
				// 主人公から柱の表面までの長さ。遠い柱は見ない
				Vector3 toPillar = pillar.center - boyPosition;
				toPillar.y = 0.0f;
				const float surfaceDistance = toPillar.Length() - pillar.radius;
				if (surfaceDistance > nearestDistance) continue;

				// どれかの悪党との間に、この柱があるか
				for (const Vector3& enemyPosition : enemyPositionList)
				{
					Vector3 toEnemy = enemyPosition - boyPosition;
					toEnemy.y = 0.0f;
					const float lengthSq = DotXZ(toEnemy, toEnemy);
					if (lengthSq <= EPSILON) continue;

					// 主人公 → 悪党の線の上で、柱の真ん中に一番近い所(0 = 主人公、1 = 悪党)
					const float rate = DotXZ(toPillar, toEnemy) / lengthSq;
					if (rate <= 0.0f || rate >= 1.0f) continue;

					// その所と柱の真ん中の長さが半径より短ければ、線が柱を通っている
					Vector3 closest = toEnemy;
					closest.Scale(rate);
					const Vector3 gap = toPillar - closest;
					if (DotXZ(gap, gap) > pillar.radius * pillar.radius) continue;

					nearestPillar = &pillar;
					nearestDistance = surfaceDistance;
					break;
				}
			}

			return nearestPillar;
		}


		EnLeanSide PhotoChance::CalcLeanSide(const Pillar& pillar, const Vector3& boyPosition, const Vector3& forwardXZ, const Vector3& rightXZ, const float edgeLimit) const
		{
			// カメラから見た柱の位置(x = 右、z = 前)
			Vector3 toPillar = pillar.center - boyPosition;
			toPillar.y = 0.0f;
			const float x = DotXZ(toPillar, rightXZ);
			const float z = DotXZ(toPillar, forwardXZ);

			// 柱が横や後ろにある時は、のぞかない
			if (z <= pillar.radius) return EnLeanSide::None;

			// 柱の左はしと右はしを、画面の横の位置にする(-1 = 画面の左端、0 = 真ん中、1 = 右端)
			const float screenWidth = z * m_tanHalfFovX;
			const float leftEdge = (x - pillar.radius) / screenWidth;
			const float rightEdge = (x + pillar.radius) / screenWidth;

			// 柱が真ん中をふさいでいなければ(はしが真ん中を越えていれば)、のぞかなくても見える
			if (leftEdge >= 0.0f || rightEdge <= 0.0f) return EnLeanSide::None;

			// はしが真ん中の近く(edgeLimit より内側)にある方から、のぞける
			const bool canRight = rightEdge <= edgeLimit;
			const bool canLeft = leftEdge >= -edgeLimit;

			// 両方のぞけるなら、はしが真ん中に近い方からのぞく
			if (canRight && canLeft) return rightEdge < -leftEdge ? EnLeanSide::Right : EnLeanSide::Left;
			if (canRight) return EnLeanSide::Right;
			if (canLeft) return EnLeanSide::Left;

			// 柱が大きくて、どちらのはしも遠い
			return EnLeanSide::None;
		}
	}
}
