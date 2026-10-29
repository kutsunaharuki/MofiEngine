/**
 * @file NavGrid.cpp
 * @brief 経路探索用の地図クラス。(通れる / 通れない)
 */
#include "stdafx.h"
#include "NavGrid.h"
#include "Source/Stage/Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include <algorithm>
#include <cfloat>
#include <cmath>


namespace app
{
	namespace ai
	{
		NavGrid::NavGrid()
			: m_origin(Vector3::Zero)
			, m_width(0)
			, m_height(0)
		{}
		
		
		NavGrid::~NavGrid()
		{}


		void NavGrid::Build(const stage::Stage& stage, const NavGridSetting & setting)
		{
			m_setting = setting;
			m_blocked.clear();

			m_width = 0;
			m_height = 0;

			/** 1.地図の広さを決める */
			Vector3 stageMin(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 stageMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			bool hasObject = false;


			for (int i = 0; i < stage.GetObjectCount(); ++i)
			{
				// StageObjectのポインタを取得
				const actor::StageObject* obj = stage.GetStageObject(i);

				// StageObjectがnullptrだったりコリジョンがない場合はスキップする
				if (obj == nullptr || !obj->IsCollision()) continue;

				// モデルを囲む箱の小さい角と大きい角を取得
				const Vector3& boxMin = obj->GetWorldAABBMin();
				const Vector3& boxMax = obj->GetWorldAABBMax();

				stageMin.x = (std::min)(stageMin.x, boxMin.x);
				stageMin.y = (std::min)(stageMin.y, boxMin.y);
				stageMin.z = (std::min)(stageMin.z, boxMin.z);
				stageMax.x = (std::max)(stageMax.x, boxMax.x);
				stageMax.y = (std::max)(stageMax.y, boxMax.y);
				stageMax.z = (std::max)(stageMax.z, boxMax.z);

				// 置物が1つでもあれば、地図を作る
				hasObject = true;
			}
			
			// 置物が1つもなければ、地図を作らない
			if (!hasObject) return;

			const float cellSize = m_setting.cellSize;
			m_origin = Vector3(stageMin.x, m_setting.floorHeight, stageMin.z);

			// 端が切れないように切り上げる。最低でも 1 マス
			m_width = (std::max)(1, static_cast<int>(std::ceil( (stageMax.x - stageMin.x) / cellSize)));
			m_height = (std::max)(1, static_cast<int>(std::ceil( (stageMax.z - stageMin.z) / cellSize)));

			// 最初は全部通れるようにする
			m_blocked.assign(static_cast<size_t>(m_width) * static_cast<size_t>(m_height), 0);


			/** 2.敵の体の高さにある置物を塞ぐ */
			const float bandMin = m_setting.floorHeight + m_setting.stepHeight;
			const float bandMax = m_setting.floorHeight + m_setting.agentHeight;

			for (int i = 0; i < stage.GetObjectCount(); ++i)
			{
				const actor::StageObject* obj = stage.GetStageObject(i);
				if (obj == nullptr || !obj->IsCollision()) continue;

				Vector3 boxMin = obj->GetWorldAABBMin();
				Vector3 boxMax = obj->GetWorldAABBMax();

				// 体の高さに重なっていなければ、塞がない
				if (boxMax.y <= bandMin || boxMin.y >= bandMax) continue;
				
				boxMin.x -= m_setting.agentRadius;
				boxMin.z -= m_setting.agentRadius;
				boxMax.x += m_setting.agentRadius;
				boxMax.z += m_setting.agentRadius;

				// 箱に重なるマスを全部塞ぐ
				BlockBox(boxMin, boxMax);
			}
		}
		
		
		bool NavGrid::WorldToCell(const Vector3 & pos, int& outX, int& outZ) const
		{
			outX = ToCellX(pos.x);
			outZ = ToCellZ(pos.z);
			return IsInside(outX,outZ);
		}
		
		
		Vector3 NavGrid::CellToWorld(const int x, const int z) const
		{
			// マスの角 + 半分のマスでマスの中心になる
			const float cellSize = m_setting.cellSize;

			return Vector3(
				m_origin.x + (static_cast<float>(x) + 0.5f) * cellSize,
				m_origin.y,
				m_origin.z + (static_cast<float>(z) + 0.5f) * cellSize
			);
		}


		int NavGrid::ToCellX(const float worldX) const
		{
			// マイナス側でも正しく切り捨てるように floor を使う(int へのキャストは 0 に向かって切り捨てる)
			return static_cast<int>(std::floor((worldX - m_origin.x) / m_setting.cellSize));
		}


		int NavGrid::ToCellZ(const float worldZ) const
		{
			// マイナス側でも正しく切り捨てるように floor を使う(int へのキャストは 0 に向かって切り捨てる)
			return static_cast<int>(std::floor((worldZ - m_origin.z) / m_setting.cellSize));
		}


		void NavGrid::BlockBox(const Vector3& boxMin, const Vector3& boxMax)
		{
			// 箱の角をマス番号にして地図の範囲に収める
			const int minX = (std::max)(0, ToCellX(boxMin.x));
			const int minZ = (std::max)(0, ToCellZ(boxMin.z));
			const int maxX = (std::min)(m_width - 1, ToCellX(boxMax.x));
			const int maxZ = (std::min)(m_height - 1, ToCellZ(boxMax.z));

			for (int z = minZ; z <= maxZ; ++z)
			{
				for (int x = minX; x <= maxX; ++x)
				{
					m_blocked[ToIndex(x, z)] = 1;
				}
			}
		}
	}
}