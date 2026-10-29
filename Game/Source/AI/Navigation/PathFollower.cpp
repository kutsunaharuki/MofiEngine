/**
 * @file PathFollower.cpp
 * @brief 道順(座標の列)を順番に辿る
 */
#include "stdafx.h"
#include "PathFollower.h"
#include "NavGrid.h"
#include <algorithm>
#include <cfloat>
#include <cmath>


namespace app
{
	namespace ai
	{
		namespace
		{
			/** 着いたとみなす距離の初期値(1マス 50 より小さくする) */
			constexpr float DEFAULT_ARRIVE_RADIUS = 30.0f;

			/** 円周率 */
			constexpr float PI = 3.14159265f;
			/** 2つ目の揺れの速さ(1つ目に対する倍率。割り切れない数にすると、同じ揺れ方をくり返さない) */
			constexpr float WANDER_SECOND_FREQUENCY_RATE = 2.3f;
			/** 2つ目の揺れのずらし方 */
			constexpr float WANDER_SECOND_PHASE_RATE = 1.7f;
			/** 2つ目の揺れの大きさ(1つ目に対する割合) */
			constexpr float WANDER_SECOND_AMPLITUDE_RATE = 0.5f;
			/** 2つの揺れを足した最大値(1 + 0.5)。これで割って -1 ~ 1 に収める */
			constexpr float WANDER_MAX_SUM = 1.0f + WANDER_SECOND_AMPLITUDE_RATE;
			/** 目指す点にこの距離(着いたとみなす距離の何倍か)より近いと、揺れを小さくし始める */
			constexpr float WANDER_FADE_DISTANCE_RATE = 2.0f;

			/** 詰まりを調べる間隔(秒) */
			constexpr float STUCK_CHECK_INTERVAL = 0.5f;
			/** 1回の間隔で、これ以上近づいていれば進んでいるとみなす距離 */
			constexpr float STUCK_MIN_PROGRESS = 5.0f;
			/** 近づけていない時間がこれを超えたら、詰まったとみなす(秒) */
			constexpr float STUCK_TIME_LIMIT = 2.0f;


			/**
			 * @brief XZ だけの距離を求める
			 * @param a 1つ目の座標
			 * @param b 2つ目の座標
			 * @return XZ の距離(高さは見ない)
			 */
			float DistanceXZ(const Vector3& a, const Vector3& b)
			{
				const float diffX = b.x - a.x;
				const float diffZ = b.z - a.z;
				return std::sqrt(diffX * diffX + diffZ * diffZ);
			}


			/**
			 * @brief 方向を Y 軸まわりに回す
			 * @param direction 方向
			 * @param angleRad 回す角度(ラジアン)
			 * @return 回した方向
			 */
			Vector3 RotateY(const Vector3& direction, const float angleRad)
			{
				const float cosA = std::cos(angleRad);
				const float sinA = std::sin(angleRad);
				return Vector3(
					direction.x * cosA - direction.z * sinA,
					0.0f,
					direction.x * sinA + direction.z * cosA
				);
			}
		}



		PathFollower::PathFollower()
			: m_navGrid(nullptr)
			, m_index(0)
			, m_direction(Vector3::Zero)
			, m_moveAmount(0.0f)
			, m_arriveRadius(DEFAULT_ARRIVE_RADIUS)
			, m_isArrived(false)
			, m_wanderTime(0.0f)
			, m_wanderPhase(0.0f)
			, m_random(std::random_device{}())
			, m_stuckCheckTimer(0.0f)
			, m_lastDistance(FLT_MAX)
			, m_stuckTime(0.0f)
			, m_isStuck(false)
		{}
		
		
		PathFollower::~PathFollower()
		{}
		
		
		void PathFollower::SetPath(const std::vector<Vector3>&path)
		{
			m_path = path;
			m_index = 0;
			m_isArrived = m_path.empty();
			m_direction = Vector3::Zero;
			m_moveAmount = 0.0f;

			// 揺れの始まりをランダムにする(同じ道順でも、敵ごとに揺れ方が変わる)
			// ※uniform_int_distribution は整数の範囲しか指定できないのでuniform_real_distributionを使用する
			std::uniform_real_distribution<float> phaseRange(0.0f, 2.0f * PI);
			m_wanderPhase = phaseRange(m_random);
			m_wanderTime = 0.0f;

			ResetStuck();
		}
		
		
		void PathFollower::Clear()
		{
			m_path.clear();
			m_index = 0;
			m_isArrived = false;
			m_direction = Vector3::Zero;
			m_moveAmount = 0.0f;
			
			ResetStuck();
		}
		
		
		void PathFollower::Update(const Vector3 & position, const float deltaTime)
		{
			// 道順が無い、または着いていたら止まる
			if (m_path.empty() || m_isArrived)
			{
				m_direction = Vector3::Zero;
				m_moveAmount = 0.0f;
				return;
			}

			/** 1.目指す点に近づいていたら、次の点に進める */
			AdvanceIndex(position);
			if (m_isArrived)
			{
				m_direction = Vector3::Zero;
				m_moveAmount = 0.0f;
				return;
			}

			/** 2.目指す点への方向を求める */
			const Vector3& target = m_path[m_index];
			Vector3 baseDirection = Vector3(target.x - position.x, 0.0f, target.z - position.z);
			if (baseDirection.LengthSq() <= 0.0f)
			{
				m_direction = Vector3::Zero;
				m_moveAmount = 0.0f;
				return;
			}
			baseDirection.Normalize();

			/** 3.ふらつかせる */
			m_wanderTime += deltaTime;
			m_direction = ApplyWander(position, baseDirection);
			m_moveAmount = 1.0f;

			/** 4.詰まっていないか調べる */
			UpdateStuck(position, deltaTime);
		}
		
		
		Vector3 PathFollower::GetCurrentTarget() const
		{
			if (m_path.empty()) return Vector3::Zero;
			return m_path[m_index];
		}
		
		
		void PathFollower::AdvanceIndex(const Vector3& position)
		{
			const int lastIndex = static_cast<int>(m_path.size()) - 1;

			// 足が速いと1フレームで複数の点を通り過ぎることがあるので、近い間は繰り返す
			while (DistanceXZ(position, m_path[m_index]) < m_arriveRadius)
			{
				// 最後の点なら着いたとみなす
				if (m_index >= lastIndex)
				{
					m_isArrived = true;
					return;
				}

				++m_index;

				// 目指す点が変わったので詰まりの計測をやり直す
				ResetStuck();
			}
		}
		
		
		Vector3 PathFollower::ApplyWander(const Vector3 & position, const Vector3 & baseDirection) const
		{
			// 揺れない設定ならそのまま
			if (m_wanderSetting.amplitudeDeg <= 0.0f) return baseDirection;

			// 速さの違う2つの sin 波を足して、規則的すぎない揺れにする(-1 ~ 1)
			const float time = m_wanderTime * m_wanderSetting.frequency * 2.0f * PI;
			const float wave =
				std::sin(time + m_wanderPhase)
				+ WANDER_SECOND_AMPLITUDE_RATE * std::sin(time * WANDER_SECOND_FREQUENCY_RATE + m_wanderPhase * WANDER_SECOND_PHASE_RATE);

			float angleRad = Math::DegToRad(m_wanderSetting.amplitudeDeg) * (wave / WANDER_MAX_SUM);

			// 目指す点に近いほど揺れを小さくする
			const float distance = DistanceXZ(position, m_path[m_index]);
			const float fadeDistance = m_arriveRadius * WANDER_FADE_DISTANCE_RATE;
			angleRad *= (std::min)(1.0f, distance / fadeDistance);

			const Vector3 wanderDirection = RotateY(baseDirection, angleRad);

			// 揺れた先が壁なら揺れを捨てて本来の方向に進む
			if (m_navGrid != nullptr)
			{
				const Vector3 probe(
					position.x + wanderDirection.x * m_wanderSetting.probeDistance,
					position.y,
					position.z + wanderDirection.z * m_wanderSetting.probeDistance
				);
				int x = 0;
				int z = 0;
				
				if (!m_navGrid->WorldToCell(probe, x, z) || m_navGrid->IsBlocked(x, z)) return baseDirection;
			}

			return wanderDirection;
		}
		
		
		void PathFollower::UpdateStuck(const Vector3& position, const float deltaTime)
		{
			m_stuckCheckTimer += deltaTime;
			if (m_stuckCheckTimer < STUCK_CHECK_INTERVAL) return;
			m_stuckCheckTimer = 0.0f;

			// 前に調べた時より目指す点に近づけているか
			const float distance = DistanceXZ(position, m_path[m_index]);

			if (m_lastDistance - distance >= STUCK_MIN_PROGRESS)
			{
				// 進めている
				m_stuckTime = 0.0f;
				m_isStuck = false;
			}
			else
			{
				// 近づけていない時間を足す
				m_stuckTime += STUCK_CHECK_INTERVAL;
				if (m_stuckTime >= STUCK_TIME_LIMIT)
				{
					m_isStuck = true;
				}
			}
			m_lastDistance = distance;
		}
		
		
		void PathFollower::ResetStuck()
		{
			m_stuckCheckTimer = 0.0f;
			m_lastDistance = FLT_MAX;
			m_stuckTime = 0.0f;
			m_isStuck = false;
		}
	}
}