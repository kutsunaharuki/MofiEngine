/**
 * @file VisitorGroup.cpp
 * @brief 訪問者(一般客)の 1グループ
 */
#include "stdafx.h"
#include "VisitorGroup.h"
#include "Visitor.h"
#include "Source/AI/Navigation/Pathfinder.h"
#include "Source/AI/Navigation/PathFollower.h"
#include <algorithm>
#include <cmath>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 扇形の数(円を 4 つに分ける) */
			constexpr int QUADRANT_NUM = 4;
			/** 扇形 1 つの角度 */
			constexpr float QUADRANT_ANGLE_DEG = 90.0f;
			/** 扇形の端に寄りすぎないための余白(角度) */
			constexpr float SECTOR_MARGIN_DEG = 10.0f;
			/** 親からの距離の平均(半径に対する割合) */
			constexpr float AVERAGE_DISTANCE_RATE = 0.6f;
			/** 親からの距離の最小(半径に対する割合。親と重ならないように) */
			constexpr float MIN_DISTANCE_RATE = 0.3f;
			/** 近すぎた時に引き直す最大の回数 */
			constexpr int MAX_RETRY = 20;
			/** グループの人数の下限と上限(1 なら 1 人の客) */
			constexpr int MEMBER_COUNT_MIN = 1;
			constexpr int MEMBER_COUNT_MAX = 8;
			/** 散らばり具合の下限(0 だと正規分布が作れない) */
			constexpr float SCATTER_MIN = 0.01f;

			/** 立ち位置に着いたとみなす距離(これより近ければ止まる) */
			constexpr float FOLLOW_ARRIVE_RADIUS = 15.0f;
			/** 立ち位置に近づいたら、ゆっくりにし始める距離 */
			constexpr float FOLLOW_SLOW_RADIUS = 80.0f;
			/** ついていく時の移動量の下限(小さすぎると止まっているとみなされる) */
			constexpr float FOLLOW_AMOUNT_MIN = 0.3f;
			/** ついていく時の移動量の上限(離れていたら少し早足で追いつく) */
			constexpr float FOLLOW_AMOUNT_MAX = 1.2f;
			/** 道順が見つからなかった時に、もう一度探すまでの時間(秒) */
			constexpr float RETRY_MOVE_TIME = 1.0f;

			/** ベビーカーのモデルの場所 */
			constexpr const char* STROLLER_MODEL_PATH = "Assets/modelData/Visitor/Family/family_models/family_models/BabyStroller/BabyStroller.tkm";

			/** SetSpawn を呼ばなかった時(試し用)に選ぶ見た目(今までの 5 人) */
			const Visitor::VisitorType DEFAULT_TYPE_LIST[] =
			{
					Visitor::VisitorType::Karen
				,	Visitor::VisitorType::OfficeMan
				,	Visitor::VisitorType::WomanAfro
				,	Visitor::VisitorType::WomanAsian
				,	Visitor::VisitorType::WomanEuro
			};
		}


		VisitorGroup::VisitorGroup()
			: m_leaderIndex(0)
			, m_random(std::random_device{}())
			, m_pathfinder(nullptr)
			, m_pathFollower(std::make_unique<ai::PathFollower>())
			, m_targetIndex(-1)
			, m_groupState(GroupState::Look)
			, m_lookTimer(0.0f)
			, m_talkTimer(0.0f)
			, m_spawnPosition(Vector3::Zero)
			, m_hasSpawn(false)
		{}


		// ※ Visitor・PathFollower を前方宣言しているので、unique_ptr を消すデストラクタは .cpp に書く
		VisitorGroup::~VisitorGroup()
		{}


		void VisitorGroup::Initialize()
		{
			m_parameter.Initialize(VISITOR_GROUP_PARAMETER_JSON_PATH);
			const VisitorGroupParameter& parameter = m_parameter.Get();

			// メンバーを作る(人数は 1 〜 8 に収める)
			if (m_hasSpawn)
			{
				// VisitorManager が決めた顔ぶれの順に作る(先頭が親)
				const int memberCount = (std::min)(static_cast<int>(m_composition.memberTypeList.size()), MEMBER_COUNT_MAX);
				for (int i = 0; i < memberCount; ++i)
				{
					m_memberList.push_back(std::make_unique<Visitor>(m_composition.memberTypeList[i]));
				}
				m_leaderIndex = 0;
			}
			else
			{
				// 試し用: JSON の人数で、今までの 5 人から等しい確率で選ぶ
				const int memberCount = TClamp<int>::Clamp(parameter.memberCount, MEMBER_COUNT_MIN, MEMBER_COUNT_MAX);
				constexpr int typeCount = static_cast<int>(sizeof(DEFAULT_TYPE_LIST) / sizeof(DEFAULT_TYPE_LIST[0]));
				std::uniform_int_distribution<int> typeRange(0, typeCount - 1);
				for (int i = 0; i < memberCount; ++i)
				{
					m_memberList.push_back(std::make_unique<Visitor>(DEFAULT_TYPE_LIST[typeRange(m_random)]));
				}
				// 真ん中の人を親にする
				m_leaderIndex = memberCount / 2;
			}

			// 顔ぶれが空でも壊れないように(誰もいない組は何もしない)
			if (m_memberList.empty()) return;

			// 立ち位置を決めて置いてから Start(キャラクターコントローラーが、置いた位置で作られる)
			MakeSlotList();
			PlaceMembers(false);
			for (auto& member : m_memberList)
			{
				member->Start();
			}

			m_layoutParameter = parameter;

			// 最初は立ち止まっているところから(グループごとに歩き出す時間がずれるように、見る時間をランダムに決める)
			StartLook();

			// ベビーカーがある組は、ベビーカーを作って親の前に置く
			if (m_composition.hasStroller)
			{
				InitStroller();
				UpdateStroller();
			}
		}


		void VisitorGroup::Update()
		{
			// 誰もいない組は何もしない(親がいないため)
			if (m_memberList.empty()) return;

			const VisitorGroupParameter& parameter = m_parameter.Get();
			const float deltaTime = util::WorldTime::GetDeltaTime();

			// ImGui で並び方の値を変えたら、並び直す(調整用)
			if (IsLayoutChanged(parameter))
			{
				ApplyLayoutChange(parameter);
				m_layoutParameter = parameter;
			}

			// 頭脳(グループ)が、メンバーの入力を決める
			// ※ メンバーの Update(ステートマシン)より先に決める
			UpdateLeader(deltaTime);
			UpdateFollowers();
			UpdateTalk(deltaTime);

			for (auto& member : m_memberList)
			{
				member->Update();
			}

			// ベビーカーは、親が動いた後に置く(先に置くと、1 フレーム遅れてずれて見える)
			UpdateStroller();
		}


		void VisitorGroup::Render(RenderContext& rc)
		{
			for (auto& member : m_memberList)
			{
				member->Render(rc);
			}

			if (m_strollerRender) m_strollerRender->Draw(rc);
		}


		void VisitorGroup::SetSpawn(const Vector3& position, const GroupComposition& composition)
		{
			m_spawnPosition = position;
			m_composition = composition;
			m_hasSpawn = true;
		}


		void VisitorGroup::SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid)
		{
			m_pathfinder = pathfinder;
			// 地図はふらつきの壁チェックに使う(今はふらつかせていない)
			m_pathFollower->SetNavGrid(navGrid);
		}


		void VisitorGroup::MakeSlotList()
		{
			const int memberCount = static_cast<int>(m_memberList.size());

			// 親の立ち位置は (0, 0, 0)
			m_slotList.assign(memberCount, Vector3::Zero);
			// 決まった立ち位置の一覧(親とも重ならないように、最初から親を入れておく)
			std::vector<Vector3> placedList = { Vector3::Zero };

			// 4 つの扇形を混ぜて、最初の 4 人に 1 つずつ配る(全部の扇形に、必ず 1 人以上入る)
			std::vector<int> quadrantList = { 0, 1, 2, 3 };
			std::shuffle(quadrantList.begin(), quadrantList.end(), m_random);
			// 5 人目からは、ランダムな扇形
			std::uniform_int_distribution<int> quadrantRange(0, QUADRANT_NUM - 1);

			// 親を除いて、何人目か
			int order = 0;
			for (int i = 0; i < memberCount; ++i)
			{
				if (i == m_leaderIndex) continue;

				const int quadrant = (order < QUADRANT_NUM) ? quadrantList[order] : quadrantRange(m_random);
				++order;

				m_slotList[i] = MakeSlot(quadrant, placedList);
				placedList.push_back(m_slotList[i]);
			}
		}


		Vector3 VisitorGroup::MakeSlot(const int quadrant, const std::vector<Vector3>& placedList)
		{
			const VisitorGroupParameter& parameter = m_parameter.Get();

			// 角度：扇形の中でランダム(端は少し空ける)
			const float startDeg = quadrant * QUADRANT_ANGLE_DEG + SECTOR_MARGIN_DEG;
			const float endDeg = (quadrant + 1) * QUADRANT_ANGLE_DEG - SECTOR_MARGIN_DEG;
			std::uniform_real_distribution<float> angleRange(startDeg, endDeg);

			// 距離：山の形(正規分布)。平均は半径の 6 割、ばらつきが scatter
			const float scatter = (std::max)(parameter.scatter, SCATTER_MIN);
			std::normal_distribution<float> distanceRange(parameter.groupRadius * AVERAGE_DISTANCE_RATE, scatter);
			const float minDistance = parameter.groupRadius * MIN_DISTANCE_RATE;

			Vector3 slot = Vector3::Zero;
			for (int retry = 0; retry < MAX_RETRY; ++retry)
			{
				const float angle = Math::DegToRad(angleRange(m_random));
				// 山のすそは長いので、半径の 3 割 〜 半径 に収める
				const float distance = TClamp<float>::Clamp(distanceRange(m_random), minDistance, parameter.groupRadius);
				slot = Vector3(std::cos(angle) * distance, 0.0f, std::sin(angle) * distance);

				if (IsFarFromOthers(slot, placedList, parameter.minSpacing)) break;
			}

			// 引き直しても見つからなければ、最後の場所で妥協する(当たり判定ができたら押し合いでずれる)
			return slot;
		}


		bool VisitorGroup::IsFarFromOthers(const Vector3& slot, const std::vector<Vector3>& placedList, const float minSpacing) const
		{
			const float minSpacingSq = minSpacing * minSpacing;
			for (const Vector3& placed : placedList)
			{
				Vector3 diff = slot - placed;
				if (diff.LengthSq() < minSpacingSq) return false;
			}
			return true;
		}


		void VisitorGroup::PlaceMembers(const bool isStarted)
		{
			const Vector3& center = GetSpawnPosition(m_parameter.Get());

			for (size_t i = 0; i < m_memberList.size(); ++i)
			{
				const Vector3 position = center + m_slotList[i];

				// Start の後は Warp(キャラクターコントローラーの位置も一緒に動かす)
				if (isStarted)
				{
					m_memberList[i]->Warp(position);
					continue;
				}
				m_memberList[i]->SetPosition(position);
			}
		}


		bool VisitorGroup::IsLayoutChanged(const VisitorGroupParameter& parameter) const
		{
			const Vector3 positionDiff = parameter.groupPosition - m_layoutParameter.groupPosition;

			const bool isRadiusChanged = parameter.groupRadius != m_layoutParameter.groupRadius;
			const bool isScatterChanged = parameter.scatter != m_layoutParameter.scatter;
			const bool isSpacingChanged = parameter.minSpacing != m_layoutParameter.minSpacing;
			// ※ SetSpawn で場所を決めた組は、JSON の groupPosition を変えても動かさない(全部の組が同じ場所に集まるため)
			const bool isPositionChanged = !m_hasSpawn && positionDiff.LengthSq() > 0.0f;

			return isRadiusChanged || isScatterChanged || isSpacingChanged || isPositionChanged;
		}


		void VisitorGroup::ApplyLayoutChange(const VisitorGroupParameter& parameter)
		{
			// 立ち位置を決め直す(ほかの人は、新しい立ち位置へ歩いていく)
			MakeSlotList();

			const Vector3 positionDiff = parameter.groupPosition - m_layoutParameter.groupPosition;
			if (m_hasSpawn || positionDiff.LengthSq() <= 0.0f) return;

			// 位置が変わった時だけ、全員をワープして、立ち止まるところからやり直す
			PlaceMembers(true);
			m_pathFollower->Clear();
			StartLook();
		}


		const Vector3& VisitorGroup::GetSpawnPosition(const VisitorGroupParameter& parameter) const
		{
			return m_hasSpawn ? m_spawnPosition : parameter.groupPosition;
		}


		void VisitorGroup::UpdateLeader(const float deltaTime)
		{
			Visitor& leader = GetLeader();

			// 立ち止まって見ている間は、時間が来たら次の美術品へ
			if (m_groupState == GroupState::Look)
			{
				leader.SetMoveInput(Vector3::Zero, 0.0f);

				m_lookTimer -= deltaTime;
				if (m_lookTimer > 0.0f) return;

				// 道順が見つからなければ、少し待ってから探し直す
				if (!StartMoveToNextTarget()) m_lookTimer = RETRY_MOVE_TIME;
				return;
			}

			// 歩いている間は、道順を辿る
			const Vector3& position = leader.GetTransform().GetWorldPosition();
			m_pathFollower->Update(position, deltaTime);

			// 詰まったら、今の位置から道順を求め直す(ほかの人とぶつかった時などの保険)
			if (m_pathFollower->IsStuck())
			{
				std::vector<Vector3> path;
				if (m_pathfinder->FindPath(position, m_targetList[m_targetIndex], path))
				{
					m_pathFollower->SetPath(path);
					m_pathFollower->Update(position, deltaTime);
				}
			}

			// 着いたら、立ち止まって見る
			if (m_pathFollower->IsArrived())
			{
				StartLook();
				leader.SetMoveInput(Vector3::Zero, 0.0f);
				return;
			}

			leader.SetMoveInput(m_pathFollower->GetDirection(), m_pathFollower->GetMoveAmount());
		}


		void VisitorGroup::UpdateFollowers()
		{
			const Vector3& leaderPosition = GetLeader().GetTransform().GetWorldPosition();
			// 立ち止まっている間は、全員で今の美術品の方を向く
			const bool isLooking = m_groupState == GroupState::Look && m_targetIndex >= 0;

			for (int i = 0; i < static_cast<int>(m_memberList.size()); ++i)
			{
				Visitor& member = *m_memberList[i];
				const Vector3& position = member.GetTransform().GetWorldPosition();

				// 向く方向(歩いている間は、進む方向を向くので 0 にしておく)
				Vector3 lookDirection = Vector3::Zero;
				if (isLooking)
				{
					lookDirection = m_targetList[m_targetIndex] - position;
					lookDirection.y = 0.0f;
				}
				member.SetLookDirection(lookDirection);

				// 親は道順を辿るので、ついていかない
				if (i == m_leaderIndex) continue;

				// 親の位置 + 自分の立ち位置 へ向かう
				Vector3 toSlot = leaderPosition + m_slotList[i] - position;
				toSlot.y = 0.0f;
				const float distance = toSlot.Length();

				// 着いていたら止まる
				if (distance < FOLLOW_ARRIVE_RADIUS)
				{
					member.SetMoveInput(Vector3::Zero, 0.0f);
					continue;
				}

				// 近いほどゆっくり、離れていたら少し早足(ぴったり止まれるように、追いつけるように)
				const float amount = TClamp<float>::Clamp(distance / FOLLOW_SLOW_RADIUS, FOLLOW_AMOUNT_MIN, FOLLOW_AMOUNT_MAX);
				toSlot.Normalize();
				member.SetMoveInput(toSlot, amount);
			}
		}


		void VisitorGroup::UpdateTalk(const float deltaTime)
		{
			m_talkTimer -= deltaTime;
			if (m_talkTimer > 0.0f) return;

			const VisitorGroupParameter& parameter = m_parameter.Get();
			m_talkTimer = parameter.talkSwitchTime;

			// 1人ひとり、話すかをくじで決める
			std::bernoulli_distribution talkRange(TClamp<float>::Clamp(parameter.talkRate, 0.0f, 1.0f));
			std::vector<bool> talkingList(m_memberList.size(), false);
			int talkingCount = 0;
			for (size_t i = 0; i < talkingList.size(); ++i)
			{
				talkingList[i] = talkRange(m_random);
				if (talkingList[i]) ++talkingCount;
			}

			// 1人だけで話していると独り言に見えるので、その時は誰も話さない
			if (talkingCount == 1) talkingList.assign(talkingList.size(), false);

			for (size_t i = 0; i < m_memberList.size(); ++i)
			{
				m_memberList[i]->SetTalking(talkingList[i]);
			}
		}


		bool VisitorGroup::StartMoveToNextTarget()
		{
			// 道順を求めるものか、美術品が無ければ歩けない(立っているだけ)
			if (m_pathfinder == nullptr || m_targetList.empty()) return false;

			// 次の美術品をランダムに選ぶ(今と同じ美術品は避ける)
			const int targetCount = static_cast<int>(m_targetList.size());
			std::uniform_int_distribution<int> targetRange(0, targetCount - 1);
			int nextIndex = targetRange(m_random);
			if (targetCount > 1 && nextIndex == m_targetIndex) nextIndex = (nextIndex + 1) % targetCount;

			// 親の位置から道順を求める
			// ※ 壁に掛かった絵は通れないマスにあるが、Pathfinder が一番近い通れるマスに置き換える
			const Vector3& position = GetLeader().GetTransform().GetWorldPosition();
			std::vector<Vector3> path;
			if (!m_pathfinder->FindPath(position, m_targetList[nextIndex], path)) return false;

			m_pathFollower->SetPath(path);
			m_targetIndex = nextIndex;
			m_groupState = GroupState::Move;
			return true;
		}


		void VisitorGroup::StartLook()
		{
			const VisitorGroupParameter& parameter = m_parameter.Get();

			// 見る時間をランダムに決める(最大が最小より小さくても壊れないように)
			const float lookTimeMax = (std::max)(parameter.lookTimeMin, parameter.lookTimeMax);
			std::uniform_real_distribution<float> lookTimeRange(parameter.lookTimeMin, lookTimeMax);

			m_lookTimer = lookTimeRange(m_random);
			m_groupState = GroupState::Look;
			m_pathFollower->Clear();
		}


		void VisitorGroup::InitStroller()
		{
			// ベビーカーは動かないので、アニメーションは無し
			m_strollerRender = std::make_unique<ModelRender>();
			m_strollerRender->Init(STROLLER_MODEL_PATH, nullptr, enModelUpAxisZ, 0);
		}


		void VisitorGroup::UpdateStroller()
		{
			// ベビーカーが無い組・誰もいない組は何もしない
			if (!m_strollerRender || m_memberList.empty()) return;

			const VisitorGroupParameter& parameter = m_parameter.Get();
			const core::TransformNode& leaderTransform = GetLeader().GetTransform();

			// 親の前の向き
			Vector3 forward = Vector3::Front;
			leaderTransform.GetWorldRotation().Apply(forward);
			forward.y = 0.0f;
			forward.Normalize();

			// 親の前に、少し離して置く
			Vector3 position = leaderTransform.GetWorldPosition();
			position += forward * parameter.strollerDistance;

			// 親と同じ向き(モデルの向きがずれていたら、JSON の角度で直す)
			Quaternion yawFix;
			yawFix.SetRotationDegY(parameter.strollerYawDeg);
			Quaternion rotation;
			rotation.Multiply(leaderTransform.GetWorldRotation(), yawFix);

			m_strollerRender->SetTRS(position, rotation, parameter.strollerScale);
			m_strollerRender->Update();
		}
	}
}
