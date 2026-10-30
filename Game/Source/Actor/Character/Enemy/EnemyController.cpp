/**
 * @file EnemyController.cpp
 * @brief 敵の入力クラス
 */
#include "stdafx.h"
#include "EnemyController.h"
#include "EnemyBase.h"
#include "Source/Stage/Stage.h"
#include "Source/Stage/StageLayout.h"
#include "Source/Actor/StageObject/StageObject.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 美術品の名前の始まり(絵画と彫刻) */
			constexpr const char* ART_NAME_PREFIX_LIST[] =
			{
				"SM_Picture",       // 絵画(額付き・絵だけ)
				"SM_OvalPainting",  // 絵画(楕円の額)
				"SM_Hebe",          // 彫刻(ヘーベー像)
				"SM_Philosopher",   // 彫刻(哲学者の像)
			};
			/** 美術品から外す名前の始まり(SM_Picture で始まるが、作品の説明プレート) */
			constexpr const char* EXCLUDE_NAME_PREFIX = "SM_PictureLabel";
			/** ステージの構造が無い時の階(全部 1F 扱い) */
			constexpr int DEFAULT_FLOOR = 0;


			/**
			 * @brief 文字列が指定した文字で始まるか
			 * @param text 文字列
			 * @param prefix 始まりの文字
			 * @return 始まっていれば true
			 */
			bool StartsWith(const std::string& text, const char* prefix)
			{
				return text.rfind(prefix, 0) == 0;
			}
		}


		EnemyController::EnemyController()
			: m_layout(nullptr)
		{}


		EnemyController::~EnemyController()
		{}


		void EnemyController::Update()
		{
			for (EnemyEntry& entry : m_enemyList)
			{
				// 階が変わった時だけ渡し直す(渡し直すと、目的地モードが最初からやり直すため)
				const int floor = ToFloorIndex(entry.enemy->GetTransform().GetWorldPosition().y);
				if (floor != entry.floor)
				{
					ApplyTargetList(entry);
				}
			}
		}


		void EnemyController::RegisterEnemy(EnemyBase* enemy)
		{
			if (enemy == nullptr) return;

			m_enemyList.push_back({ enemy, 0 });
			ApplyTargetList(m_enemyList.back());
		}


		int EnemyController::CollectArtTargets(const stage::Stage& stage)
		{
			m_targetList.clear();

			for (int i = 0; i < stage.GetObjectCount(); ++i)
			{
				const StageObject* object = stage.GetStageObject(i);
				if (object == nullptr || !IsArt(object->GetName())) continue;

				// 行き先は囲む球の中心(モデルの原点が端にある物もあるため)
				// ※ 壁に掛かった絵は通れないマスにあるが、Pathfinder が一番近い通れるマスに置き換える
				// 階は箱の下の端で決める(中心だと、背の高い絵が上の階に入ってしまう)
				const int floor = ToFloorIndex(object->GetWorldAABBMin().y);
				m_targetList.push_back({ object->GetBoundingCenter(), floor });
			}

			ApplyAllTargetList();
			return static_cast<int>(m_targetList.size());
		}


		void EnemyController::AddTarget(const Vector3& position)
		{
			m_targetList.push_back({ position, ToFloorIndex(position.y) });
			ApplyAllTargetList();
		}


		void EnemyController::ClearTargets()
		{
			m_targetList.clear();
			ApplyAllTargetList();
		}


		std::vector<Vector3> EnemyController::MakeFloorTargetList(const int floor) const
		{
			std::vector<Vector3> floorTargetList;
			for (const ArtTarget& target : m_targetList)
			{
				if (target.floor != floor) continue;
				floorTargetList.push_back(target.position);
			}
			return floorTargetList;
		}


		int EnemyController::ToFloorIndex(const float height) const
		{
			if (m_layout == nullptr || m_layout->GetFloorCount() == 0) return DEFAULT_FLOOR;
			return m_layout->FindFloorIndex(height);
		}


		bool EnemyController::IsArt(const std::string& name)
		{
			if (StartsWith(name, EXCLUDE_NAME_PREFIX)) return false;

			for (const char* prefix : ART_NAME_PREFIX_LIST)
			{
				if (StartsWith(name, prefix)) return true;
			}
			return false;
		}


		void EnemyController::ApplyTargetList(EnemyEntry& entry)
		{
			// 一覧を渡し直すと、敵は次の美術品を選ぶところからやり直す
			entry.floor = ToFloorIndex(entry.enemy->GetTransform().GetWorldPosition().y);
			entry.enemy->SetTargetList(MakeFloorTargetList(entry.floor));
		}


		void EnemyController::ApplyAllTargetList()
		{
			for (EnemyEntry& entry : m_enemyList)
			{
				ApplyTargetList(entry);
			}
		}
	}
}
