/**
 * @file EnemyAlertMode.cpp
 * @brief 敵の警戒モード(隠れ場所を見て回る頭)
 */
#include "stdafx.h"
#include "EnemyAlertMode.h"
#include "EnemyStateMachine.h"
#include "Source/AI/WeightedRandom.h"
#include "Source/AI/Navigation/PathFollower.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 隠れ場所が決まっていない */
			constexpr int NO_POINT = -1;
		}


		EnemyAlertMode::EnemyAlertMode(EnemyStateMachine* stateMachine)
			: m_stateMachine(stateMachine)
			, m_pointIndex(NO_POINT)
			, m_phase(Phase::NoPoint)
			, m_phaseTimer(0.0f)
			, m_searchIndex(0)
			, m_random(std::random_device{}())
		{}
		
		
		EnemyAlertMode::~EnemyAlertMode()
		{}
		
		
		void EnemyAlertMode::Update(const float deltaTime)
		{
			// 警戒していないなら何もしない
			if (!m_stateMachine->IsAlert()) return;

			m_phaseTimer += deltaTime;

			switch (m_phase)
			{
				// 隠れ場所が決まっていない
				case Phase::NoPoint:
				{
					// 隠れ場所があれば乱数で選んで向かい始める
					if (m_hidePointList.empty()) break;
					// 次の隠れ場所を見に行く
					ChooseNextPoint();
					StartMoveToPoint();
					break;
				}

				// 隠れ場所へ向かっている
				case Phase::MoveToPoint:
				{
					// 着いたら見回す
					if (m_stateMachine->IsArrived())
					{
						m_stateMachine->StopPathFollow();
						ChangePhase(Phase::LookAround);
						break;
					}
					// 道が見つからなかったときは、少し待ってから次の隠れ場所を探す
					if (!m_stateMachine->IsFollowingPath() && m_phaseTimer >= m_setting.retryInterval)
					{
						ChooseNextPoint();
						StartMoveToPoint();
					}
					break;
				}

				// 隠れ場所で辺りを見回す
				case Phase::LookAround:
				{
					// 見回し終わったら次の隠れ場所に向かう
					if (m_phaseTimer >= m_setting.lookAroundTime)
					{
						ChooseNextPoint();
						StartMoveToPoint();
					}
					break;
				}

				// 調べに行く場所へ向かっている(音がした所・見失った所)
				case Phase::MoveToInvestigate:
				{
					// 着いたら見回す
					if (m_stateMachine->IsArrived())
					{
						m_stateMachine->StopPathFollow();
						ChangePhase(Phase::LookAroundInvestigate);
						break;
					}
					// 道が見つからなかった時は、少し待ってからあきらめる(いつもの動きに戻る)
					if (!m_stateMachine->IsFollowingPath() && m_phaseTimer >= m_setting.retryInterval)
					{
						EndAlert();
					}
					break;
				}

				// 調べに行った場所で見回している(警戒ステートの Alert アニメーションになる)
				case Phase::LookAroundInvestigate:
				{
					// 見回し終わったら、警戒をやめていつもの動きに戻る
					if (m_phaseTimer >= m_setting.lookAroundTime)
					{
						EndAlert();
					}
					break;
				}

				// 探し回っている途中で、次の隠れ場所へ向かっている
				case Phase::MoveToSearch:
				{
					// 着いたら見回す
					if (m_stateMachine->IsArrived())
					{
						m_stateMachine->StopPathFollow();
						ChangePhase(Phase::LookAroundSearch);
						break;
					}
					// 道が見つからなかった時は、少し待ってから次の隠れ場所へ
					if (!m_stateMachine->IsFollowingPath() && m_phaseTimer >= m_setting.retryInterval)
					{
						++m_searchIndex;
						StartMoveToSearchPoint();
					}
					break;
				}

				// 探し回っている途中で、隠れ場所で見回している
				case Phase::LookAroundSearch:
				{
					// 見回し終わったら、次の隠れ場所へ(全部まわったら警戒をやめる)
					if (m_phaseTimer >= m_setting.lookAroundTime)
					{
						++m_searchIndex;
						StartMoveToSearchPoint();
					}
					break;
				}
			}
		}


		void EnemyAlertMode::StartSearchAround(const Vector3& position)
		{
			// 見失った所から近い隠れ場所を集める
			struct Candidate
			{
				Vector3 position;
				float distanceSq;
			};
			std::vector<Candidate> candidateList;
			const float radiusSq = m_setting.searchRadius * m_setting.searchRadius;
			for (const stage::HidePoint& hidePoint : m_hidePointList)
			{
				const Vector3 toPoint = hidePoint.position - position;
				const float distanceSq = toPoint.LengthSq();
				if (distanceSq > radiusSq) continue;
				candidateList.push_back({ hidePoint.position, distanceSq });
			}

			// 近くに隠れ場所が無ければ、見失った所を調べに行くだけにする
			if (candidateList.empty())
			{
				StartInvestigate(position);
				return;
			}

			// 近い順に並べて、決まった数だけ見に行く
			std::sort(candidateList.begin(), candidateList.end(),
				[](const Candidate& a, const Candidate& b) { return a.distanceSq < b.distanceSq; });
			m_searchPositionList.clear();
			for (const Candidate& candidate : candidateList)
			{
				if (static_cast<int>(m_searchPositionList.size()) >= m_setting.searchPointCount) break;
				m_searchPositionList.push_back(candidate.position);
			}

			// 盗んでいる途中でもやめて、歩く速さで探す
			m_stateMachine->SetStealing(false);
			m_stateMachine->SetAlert(true);
			m_stateMachine->SetInvestigating(true);
			m_stateMachine->SetWanderSetting(ai::WanderSetting());
			m_pointIndex = NO_POINT;

			m_searchIndex = 0;
			StartMoveToSearchPoint();
		}


		void EnemyAlertMode::StartMoveToSearchPoint()
		{
			// 全部まわったら、警戒をやめていつもの動きに戻る
			if (m_searchIndex >= static_cast<int>(m_searchPositionList.size()))
			{
				EndAlert();
				return;
			}

			// 道が見つからなくても MoveToSearch にする(更新処理で少し待ってから次を試す)
			m_stateMachine->MoveTo(m_searchPositionList[m_searchIndex]);
			ChangePhase(Phase::MoveToSearch);
		}


		void EnemyAlertMode::StartInvestigate(const Vector3& position)
		{
			// 盗んでいる途中でもやめて、見に行く(警戒の速さではなく、歩く速さで向かう)
			m_stateMachine->SetStealing(false);
			m_stateMachine->SetAlert(true);
			m_stateMachine->SetInvestigating(true);
			m_pointIndex = NO_POINT;

			// ふらつかずに、調べに行く場所へまっすぐ向かう
			m_stateMachine->SetWanderSetting(ai::WanderSetting());
			m_stateMachine->MoveTo(position);
			ChangePhase(Phase::MoveToInvestigate);
		}
		
		
		void EnemyAlertMode::StartAlert()
		{
			// もう警戒中なら何もしない(今の行き先はそのまま)
			if (m_stateMachine->IsAlert()) return;

			// 盗んでいる途中でも警戒モードに入るようにする
			m_stateMachine->SetStealing(false);
			m_stateMachine->SetAlert(true);
			m_stateMachine->SetInvestigating(false);

			// 隠れ場所が決まっていない状態
			m_pointIndex = NO_POINT;
			ChangePhase(Phase::NoPoint);
		}
		
		
		void EnemyAlertMode::EndAlert()
		{
			// 警戒していない時に呼ばれたら何もしない(目的地モードの道順を止めてしまうため)
			if (!m_stateMachine->IsAlert()) return;

			m_stateMachine->StopPathFollow();
			m_stateMachine->SetAlert(false);
			m_stateMachine->SetInvestigating(false);

			// 隠れ場所が決まっていない状態
			m_pointIndex = NO_POINT;
			ChangePhase(Phase::NoPoint);
		}
		
		
		void EnemyAlertMode::SetHidePointList(const std::vector<stage::HidePoint>& hidePointList)
		{
			m_hidePointList = hidePointList;

			// くじ引きに渡す重みの一覧を作っておく(同じ順番)
			m_weightList.clear();
			m_weightList.reserve(hidePointList.size());
			for (const stage::HidePoint& hidePoint : hidePointList)
			{
				m_weightList.push_back(hidePoint.weight);
			}


			// 番号が変わるので今の行き先は一旦忘れる
			m_pointIndex = NO_POINT;

			// 警戒中なら、新しい一覧から選び直す(階を移った時等)
			// ※ 調べに行っている途中は、隠れ場所を使っていないのでそのまま
			if (m_stateMachine->IsAlert() && !IsInvestigating())
			{
				m_stateMachine->StopPathFollow();
				ChangePhase(Phase::NoPoint);
			}
		}
		
		
		const char* EnemyAlertMode::GetPhaseName() const
		{
			switch (m_phase)
			{
			case Phase::NoPoint:     return "NoPoint";
			case Phase::MoveToPoint: return "MoveToPoint";
			case Phase::LookAround:  return "LookAround";
			case Phase::MoveToInvestigate:     return "MoveToInvestigate";
			case Phase::LookAroundInvestigate: return "LookAroundInvestigate";
			case Phase::MoveToSearch:          return "MoveToSearch";
			case Phase::LookAroundSearch:      return "LookAroundSearch";
			}

			// Phase 以外ならない
			return "Unknown";
		}
		
		
		void EnemyAlertMode::ChangePhase(const Phase phase)
		{
			m_phase = phase;
			m_phaseTimer = 0.0f;
		}
		
		
		void EnemyAlertMode::ChooseNextPoint()
		{
			// 今いる場所を除いて、重み付きくじを引く
			int nextIndex = ai::PickWeighted(m_weightList, m_pointIndex, m_random);

			// 隠れ場所が 1つしかないときは除くと選べないので、除かずに引き直す
			if (nextIndex == NO_POINT)
			{
				nextIndex = ai::PickWeighted(m_weightList, NO_POINT, m_random);
			}

			m_pointIndex = nextIndex;
		}
		
		
		void EnemyAlertMode::StartMoveToPoint()
		{
			if (m_pointIndex == NO_POINT)
			{
				ChangePhase(Phase::NoPoint);
				return;
			}

			// 警戒中はふらつかずにまっすぐに向かう
			m_stateMachine->SetWanderSetting(ai::WanderSetting());

			// 道が見つからなくても Phase を MoveToPoint にする(更新処理で少し待ってから次を試す)
			m_stateMachine->MoveTo(m_hidePointList[m_pointIndex].position);
			ChangePhase(Phase::MoveToPoint);
		}
	}
}