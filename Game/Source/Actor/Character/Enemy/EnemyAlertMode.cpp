/**
 * @file EnemyAlertMode.cpp
 * @brief 敵の警戒モード(隠れ場所を見て回る頭)
 */
#include "stdafx.h"
#include "EnemyAlertMode.h"
#include "EnemyStateMachine.h"
#include "Source/AI/WeightedRandom.h"
#include "Source/AI/Navigation/PathFollower.h"


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
			, m_alertTimer(0.0f)
			, m_random(std::random_device{}())
		{}
		
		
		EnemyAlertMode::~EnemyAlertMode()
		{}
		
		
		void EnemyAlertMode::Update(const float deltaTime)
		{
			// 警戒していないなら何もしない
			if (!m_stateMachine->IsAlert()) return;

			// 警戒の時間が終わったらやめる
			m_alertTimer += deltaTime;

			if (m_alertTimer >= m_setting.alertDuration)
			{
				// 警戒終了
				EndAlert();
				return;
			}

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
			}
		}
		
		
		void EnemyAlertMode::StartAlert()
		{
			// 警戒の時間を最初からにする
			m_alertTimer = 0.0f;
			// もう警戒中なら時間を戻すだけ(今の行き先はそのまま)
			if (m_stateMachine->IsAlert()) return;

			// 盗んでいる途中でも警戒モードに入るようにする
			m_stateMachine->SetStealing(false);
			m_stateMachine->SetAlert(true);

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
			if (m_stateMachine->IsAlert())
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
			}

			// Phase 以外ならない
			return "Unknown";
		}
		
		
		float EnemyAlertMode::GetRemainTime() const
		{
			const float remain = m_setting.alertDuration - m_alertTimer;
			// remain が 0 より大きかったら true の時は remain、falseの時は 0
			return (remain > 0.0f) ? remain : 0.0f;
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