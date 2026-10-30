/**
 * @file EnemyDestinationMode.cpp
 * @brief 敵の目的地モード(美術品を盗みに行く頭)
 */
#include "stdafx.h"
#include "EnemyDestinationMode.h"
#include "EnemyStateMachine.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/AI/Navigation/PathFollower.h"
#include <cstdlib>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 寄り道する場所を探す時に、何回まで試すか */
			constexpr int DETOUR_SEARCH_TRY_MAX = 10;
			/** 寄り道する場所は、今の位置からこのマス数以上離れた所にする(近すぎると寄り道に見えない) */
			constexpr int DETOUR_MIN_DISTANCE_CELL = 2;
			/** 美術品が決まっていない */
			constexpr int NO_TARGET = -1;
		}


		EnemyDestinationMode::EnemyDestinationMode(EnemyStateMachine* stateMachine)
			: m_stateMachine(stateMachine)
			, m_targetIndex(NO_TARGET)
			, m_phase(Phase::NoTarget)
			, m_phaseTimer(0.0f)
			, m_detourTimer(0.0f)
			, m_detourCount(0)
			, m_random(std::random_device{}())
		{}


		EnemyDestinationMode::~EnemyDestinationMode()
		{}


		void EnemyDestinationMode::Update(const float deltaTime)
		{
			// 警戒中は何もしない(警戒モードに任せる)
			if (m_stateMachine->IsAlert()) return;

			m_phaseTimer += deltaTime;

			switch (m_phase)
			{
			// 美術品が無い
			case Phase::NoTarget:
			{
				// 美術品が設定されたら、選んで向かい始める
				if (m_targetList.empty()) break;
				ChooseNextTarget();
				StartMoveToTarget();
				break;
			}

			// 美術品へ向かっている
			case Phase::MoveToTarget:
			{
				// 着いたら盗み始める
				if (m_stateMachine->IsArrived())
				{
					m_stateMachine->StopPathFollow();
					m_stateMachine->SetStealing(true);
					ChangePhase(Phase::Steal);
					break;
				}

				// 道が見つからなかった時は、少し待ってから次の美術品を試す(毎フレーム A* しないように)
				if (!m_stateMachine->IsFollowingPath())
				{
					if (m_phaseTimer >= m_setting.retryInterval)
					{
						ChooseNextTarget();
						StartMoveToTarget();
					}
					break;
				}

				// 時々、寄り道するかを決める
				m_detourTimer += deltaTime;
				if (m_detourTimer >= m_setting.detourCheckInterval)
				{
					m_detourTimer = 0.0f;

					std::uniform_real_distribution<float> chanceRange(0.0f, 1.0f);
					if (m_detourCount < m_setting.maxDetourPerTarget && chanceRange(m_random) < m_setting.detourChance)
					{
						StartDetour();
					}
				}
				break;
			}

			// 寄り道している
			case Phase::Detour:
			{
				// 着いたら(または道を見失ったら)、その場で見回す
				if (m_stateMachine->IsArrived() || !m_stateMachine->IsFollowingPath())
				{
					m_stateMachine->StopPathFollow();
					ChangePhase(Phase::LookAround);
				}
				break;
			}

			// 寄り道した先で見回している(移動量 0 なので、待機ステートになる)
			case Phase::LookAround:
			{
				// 見回し終わったら、美術品へ向かうのに戻る(今の位置から道順を求め直す)
				if (m_phaseTimer >= m_setting.lookAroundTime)
				{
					StartMoveToTarget();
				}
				break;
			}

			// 美術品を盗んでいる(盗むステートになる)
			case Phase::Steal:
			{
				// 盗み終わったら、次の美術品へ
				if (m_phaseTimer >= m_setting.stealTime)
				{
					m_stateMachine->SetStealing(false);
					ChooseNextTarget();
					StartMoveToTarget();
				}
				break;
			}
			}
		}


		void EnemyDestinationMode::SetTargetList(const std::vector<Vector3>& targetList)
		{
			m_targetList = targetList;
			m_targetIndex = NO_TARGET;

			// 今の動きをやめて、最初からやり直す(次の Update で美術品を選び直す)
			m_stateMachine->StopPathFollow();
			m_stateMachine->SetStealing(false);
			ChangePhase(Phase::NoTarget);
		}


		const char* EnemyDestinationMode::GetPhaseName() const
		{
			switch (m_phase)
			{
			case Phase::NoTarget:     return "NoTarget";
			case Phase::MoveToTarget: return "MoveToTarget";
			case Phase::Detour:       return "Detour";
			case Phase::LookAround:   return "LookAround";
			case Phase::Steal:        return "Steal";
			}
			return "Unknown";
		}


		void EnemyDestinationMode::ChangePhase(const Phase phase)
		{
			m_phase = phase;
			m_phaseTimer = 0.0f;
		}


		void EnemyDestinationMode::ChooseNextTarget()
		{
			const int targetCount = static_cast<int>(m_targetList.size());
			if (targetCount == 0)
			{
				m_targetIndex = NO_TARGET;
				return;
			}

			if (targetCount == 1)
			{
				m_targetIndex = 0;
			}
			else
			{
				// 今と違う美術品を選ぶ(1 ~ 数-1 だけずらすと、必ず今と違う番号になる)
				std::uniform_int_distribution<int> shiftRange(1, targetCount - 1);
				const int current = (m_targetIndex == NO_TARGET) ? 0 : m_targetIndex;
				m_targetIndex = (current + shiftRange(m_random)) % targetCount;
			}

			// 新しい美術品へ向かうので、寄り道の回数を0に戻す
			m_detourCount = 0;
			m_detourTimer = 0.0f;
		}


		void EnemyDestinationMode::StartMoveToTarget()
		{
			if (m_targetIndex == NO_TARGET)
			{
				ChangePhase(Phase::NoTarget);
				return;
			}

			// 目的地モードは、ふらつきながら歩く
			ai::WanderSetting wander;
			wander.amplitudeDeg = m_setting.wanderAmplitudeDeg;
			wander.frequency = m_setting.wanderFrequency;
			m_stateMachine->SetWanderSetting(wander);

			// 道が見つからなくても MoveToTarget にする(Update で少し待ってから次を試す)
			m_stateMachine->MoveTo(m_targetList[m_targetIndex]);
			ChangePhase(Phase::MoveToTarget);
		}


		bool EnemyDestinationMode::StartDetour()
		{
			Vector3 detourPosition;
			if (!FindDetourPoint(detourPosition)) return false;

			// ※ MoveTo は失敗すると止まってしまうので、失敗したら美術品へ向かい直す
			if (!m_stateMachine->MoveTo(detourPosition))
			{
				StartMoveToTarget();
				return false;
			}

			++m_detourCount;
			ChangePhase(Phase::Detour);
			return true;
		}


		bool EnemyDestinationMode::FindDetourPoint(Vector3& outPosition)
		{
			const ai::NavGrid* navGrid = m_stateMachine->GetNavGrid();
			if (navGrid == nullptr) return false;

			int cellX = 0;
			int cellZ = 0;
			if (!navGrid->WorldToCell(m_stateMachine->GetTransform().GetWorldPosition(), cellX, cellZ)) return false;

			// 今の位置のまわりの通れるマスを、ランダムに何回か試す
			const int radius = m_setting.detourRadiusCell;
			std::uniform_int_distribution<int> offsetRange(-radius, radius);
			for (int i = 0; i < DETOUR_SEARCH_TRY_MAX; ++i)
			{
				const int offsetX = offsetRange(m_random);
				const int offsetZ = offsetRange(m_random);

				// 近すぎる場所は寄り道に見えないので飛ばす
				if (std::abs(offsetX) + std::abs(offsetZ) < DETOUR_MIN_DISTANCE_CELL) continue;
				if (navGrid->IsBlocked(cellX + offsetX, cellZ + offsetZ)) continue;

				outPosition = navGrid->CellToWorld(cellX + offsetX, cellZ + offsetZ);
				return true;
			}
			return false;
		}
	}
}
