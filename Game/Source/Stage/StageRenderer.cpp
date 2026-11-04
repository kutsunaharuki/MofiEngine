/**
 * @file StageRenderer.cpp
 * @brief モデルの種類ごとにグループ分けして描く
 */
#include "stdafx.h"
#include "StageRenderer.h"
#include "Source/Actor/StageObject/StageObject.h"


namespace app
{
	namespace stage
	{
		namespace
		{
			/**
			 * グループの最大数の最小値
			 * ※ 1個しか無いモデルでも、エディターで複製するたびに作り直さないように、少し余裕を持たせる
			 */
			constexpr int MIN_CAPACITY = 16;
		}


		StageRenderer::StageRenderer()
		{}


		StageRenderer::~StageRenderer()
		{}


		void StageRenderer::Reserve(const std::string& modelPath, const bool isMirrored, const int count)
		{
			const GroupKey key(modelPath, isMirrored);
			Group& group = FindOrCreateGroup(key, count);

			// すでにあるグループで足りなければ、作り直す
			if (group.count + count > group.capacity)
			{
				InitGroup(key, group, group.count + count);
			}
		}


		InstancedModelRender& StageRenderer::Register(const std::string& modelPath, const bool isMirrored)
		{
			const GroupKey key(modelPath, isMirrored);
			Group& group = FindOrCreateGroup(key, 1);

			group.count++;
			// 最大数を超えたら、倍の大きさで作り直す(1個ずつ増やすと、追加のたびに作り直すことになる)
			if (group.count > group.capacity)
			{
				InitGroup(key, group, group.count * 2);
			}
			return group.render;
		}


		void StageRenderer::Unregister(const std::string& modelPath, const bool isMirrored)
		{
			auto it = m_groupMap.find(GroupKey(modelPath, isMirrored));
			if (it == m_groupMap.end()) return;

			Group& group = *it->second;
			if (group.count > 0) group.count--;
		}


		void StageRenderer::Render(RenderContext& rc, const std::vector<std::unique_ptr<actor::StageObject>>& objectList)
		{
			/** 1. 全グループの、今フレームの行列リストを空にする */
			for (auto& pair : m_groupMap)
			{
				pair.second->render.ClearInstance();
			}

			/** 2. 視錐台の範囲内にある置物だけ、自分のグループに行列を追加する */
			// このフレームのカメラの映る範囲(視錐台)
			Frustum frustum;
			frustum.Update(g_camera3D->GetViewProjectionMatrix());

			int visibleCount = 0;
			for (const auto& object : objectList)
			{
				// 映る範囲外なら追加しない(G-Buffer にもファインダーにも描かれない)
				if (!frustum.IsInside(object->GetBoundingCenter(), object->GetBoundingRadius())) continue;

				object->GetInstancedRender()->AddInstance(object->GetWorldMatrix());
				visibleCount++;
			}

			/** 3. グループごとに、まとめて描画に登録する(映っている物が無いグループは登録されない) */
			for (auto& pair : m_groupMap)
			{
				pair.second->render.Draw(rc);
			}

			// Profiler に描いた数を出す
			K2_PROFILE_COUNTER("Stage visible", visibleCount);
			K2_PROFILE_COUNTER("Stage groups", static_cast<int>(m_groupMap.size()));
		}


		void StageRenderer::Clear()
		{
			m_groupMap.clear();
		}


		StageRenderer::Group& StageRenderer::FindOrCreateGroup(const GroupKey& key, const int capacity)
		{
			auto it = m_groupMap.find(key);
			if (it != m_groupMap.end()) return *it->second;

			// 無ければ作る
			auto group = std::make_unique<Group>();
			InitGroup(key, *group, capacity);

			Group& result = *group;
			m_groupMap.emplace(key, std::move(group));
			return result;
		}


		void StageRenderer::InitGroup(const GroupKey& key, Group& group, const int capacity)
		{
			group.capacity = (std::max)(capacity, MIN_CAPACITY);

			// 反転(スケールがマイナス)していると、ポリゴンの表と裏が入れ替わるので、表側を描かない設定にする
			const D3D12_CULL_MODE cullMode = key.second ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;
			group.render.Init(key.first.c_str(), group.capacity, enModelUpAxisZ, cullMode);
		}
	}
}
