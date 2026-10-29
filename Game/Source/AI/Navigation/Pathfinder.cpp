/**
 * @file Pathfinder.cpp
 * @brief A* で、スタートからゴールまでの道順を求める
 */
#include "stdafx.h"
#include "Pathfinder.h"
#include "NavGrid.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <cfloat>
#include <climits>


namespace app
{
	namespace ai
	{
		namespace
		{
			/** 進める向きの数(上下右左・斜め4方向) */
			constexpr int DIRECTION_NUM = 8;
			/** 進む向き(横) */
			constexpr int DIRECTION_X[DIRECTION_NUM] = { 1,-1,0,0,1,1,-1,-1 };
			/** 進む向き(奥) */
			constexpr int DIRECTION_Z[DIRECTION_NUM] = { 0,0,1,-1,1,-1,1,-1 };

			/** まっすぐ1マス進むコスト */
			constexpr float STRAIGHT_COST = 1.0f;
			/** 斜めに1マス進むコスト(√2) */
			constexpr float DIAGONAL_COST = 1.41421356237f;

			/** 通れるマスを探すときに、何マス先まで探すか */
			constexpr int NEAREST_SEARCH_RADIUS = 10;
			/** まっすぐ行けるかを調べるときに、調べる間隔(1マスに対する割合) */
			constexpr float LINE_CHECK_STEP_RATE = 0.25f;

			/** 親がいない(スタートのマス) */
			constexpr int NO_PARENT = -1;


			/**
			 * @brief ゴールまでの予想コストを求める(8方向の最短距離)
			 * @param x 今のマスの横の番号
			 * @param z 今のマスの奥の番号
			 * @param goalX ゴールの横の番号
			 * @param goalZ ゴールの奥の番号
			 * @return 予想コスト
			 * @details 斜めに進めるだけ進んで、残りをまっすぐ進んだ時のコスト。
			 *          実際のコストより大きくならないので、最短の道順が求まる。
			 */
			float CalcHeuristic(const int x, const int z, const int goalX, const int goalZ)
			{
				const int distX = std::abs(goalX - x);
				const int distZ = std::abs(goalZ - z);
				const int diagonal = (std::min)(distX, distZ);
				const int straight = (std::max)(distX, distZ) - diagonal;
				return DIAGONAL_COST * static_cast<float>(diagonal) + STRAIGHT_COST * static_cast<float>(straight);
			}

			/** 調べる候補 */
			struct OpenNode
			{
				/** スタートからのコスト + ゴールまでの予想コスト */
				float score;
				/** マスの配列の番号 */
				int index;
			};

			/** 候補の並べ方(scoreが小さい方を先に取り出す) */
			struct OpenNodeGreater
			{
				bool operator()(const OpenNode& a, const OpenNode& b) const
				{
					return a.score > b.score;
				}
			};
		}




		/***************************************************/


		Pathfinder::Pathfinder(const NavGrid& navGrid)
			: m_navGrid(navGrid)
			, m_isSmoothing(true)
		{}
		
		
		Pathfinder::~Pathfinder()
		{}


		bool Pathfinder::FindPath(const Vector3 & start, const Vector3 & goal, std::vector<Vector3>& outPath) const
		{
			outPath.clear();

			if (!m_navGrid.IsBuilt()) return false;

			/** 1.スタートとゴールのマスを決める */
			int startX = 0;
			int startZ = 0;
			int goalX = 0;
			int goalZ = 0;

			// 地図の外にいるときは探さない
			if (!m_navGrid.WorldToCell(start, startX, startZ)) return false;
			if (!m_navGrid.WorldToCell(goal, goalX, goalZ)) return false;

			// 元のマスが通れるかを覚えておく
			const bool isStartOpen = !m_navGrid.IsBlocked(startX, startZ);
			const bool isGoalOpen = !m_navGrid.IsBlocked(goalX, goalZ);

			// 通れないマスなら、一番近い通れるマスに置き換える
			if (!FindNearestOpenCell(startX, startZ)) return false;
			if (!FindNearestOpenCell(goalX, goalZ)) return false;

			
			/** 2.A* で探す */
			const int width = m_navGrid.GetWidth();
			const size_t cellCount = static_cast<size_t>(width) * static_cast<size_t>(m_navGrid.GetHeight());

			// マスの数だけ用意する
			// スタートからのコスト
			std::vector<float> costList(cellCount, FLT_MAX);
			// どのマスから来たか
			std::vector<int> parentList(cellCount, NO_PARENT);
			// 調べ終わったか
			std::vector<uint8_t> closedList(cellCount, 0);
			/** 次に調べる候補(score が小さい順に出てくる) */
			std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> openList;

			const int startIndex = startZ * width + startX;
			const int goalIndex = goalZ * width + goalX;

			costList[startIndex] = 0.0f;
			openList.push({ CalcHeuristic(startX,startZ,goalX,goalZ), startIndex });

			bool isFound = false;

			// 調べる候補がなくなるまで繰り返す
			while (!openList.empty())
			{
				const OpenNode node = openList.top();
				openList.pop();

				// 同じマスが何回も候補に入る事があるため、調べ終わっていたらスキップする
				if (closedList[node.index] != 0) continue;
				closedList[node.index] = 1;

				// ゴールに付いたら終了
				if (node.index == goalIndex)
				{
					isFound = true;
					break;
				}

				const int x = node.index % width;
				const int z = node.index / width;

				for (int dir = 0; dir < DIRECTION_NUM; ++dir)
				{
					const int dirX = DIRECTION_X[dir];
					const int dirZ = DIRECTION_Z[dir];

					if (!CanMove(x, z, dirX, dirZ)) continue;

					const int nextIndex = (z + dirZ) * width + (x + dirX);
					if (closedList[nextIndex] != 0) continue;

					const bool isDiagonal = (dirX != 0 && dirZ != 0);
					const float nextCost = costList[node.index] + (isDiagonal ? DIAGONAL_COST : STRAIGHT_COST);

					// 今までより安く行けるときだけ、書き換えて候補に入れる
					if(nextCost < costList[nextIndex])
					{
						costList[nextIndex] = nextCost;
						parentList[nextIndex] = node.index;
						openList.push({ nextCost + CalcHeuristic(x + dirX, z + dirZ, goalX, goalZ), nextIndex });
					}
				}
			}

			// たどり着けなかった
			if (!isFound) return false;

			/** 3.ゴールから親を辿って道順にする */
			std::vector<Vector3> path;
			for (int index = goalIndex; index != NO_PARENT; index = parentList[index])
			{
				path.push_back(m_navGrid.CellToWorld(index % width, index / width));
			}
			// ゴールから集めたので逆にしてスタートからにする
			std::reverse(path.begin(), path.end());

			// 元のマスが通れるときは、マスの中心ではなく本当の座標を使う
			if (isStartOpen)
			{
				path.front().x = start.x;
				path.front().z = start.z;
			}
			if (isGoalOpen)
			{
				path.back().x = goal.x;
				path.back().z = goal.z;
			}

			/** 4. まっすぐにして今いる座標を外す */
			if (m_isSmoothing)
			{
				SmoothPath(path);
			}

			// 今いる位置は行き先ではないので外す
			// ※スタートが通れないマスだったときは、まず通れるマスに出る必要があるので残す
			if (isStartOpen && path.size() > 1)
			{
				path.erase(path.begin());
			}

			outPath = std::move(path);
			return true;
		}
		
		
		bool Pathfinder::CanMove(const int x, const int z, const int dirX, const int dirZ) const
		{
			// 進む先が通れない(範囲外も含む)
			if (m_navGrid.IsBlocked(x + dirX, z + dirZ)) return false;

			// 斜めの時は、縦と横の両方が空いていないと進めない(壁の角をすり抜けないようにする)
			if (dirX != 0 && dirZ != 0)
			{
				if (m_navGrid.IsBlocked(x + dirX, z)) return false;
				if (m_navGrid.IsBlocked(x, z + dirZ)) return false;
			}
			return true;
		}
		
		
		bool Pathfinder::FindNearestOpenCell(int& x, int& z) const
		{
			// 元のマスが通れるなら、そのまま
			if (!m_navGrid.IsBlocked(x, z)) return true;

			// 元のマスを囲む輪を、1マスずつ広げながら探す
			for (int radius = 1; radius <= NEAREST_SEARCH_RADIUS; ++radius)
			{
				int bestX = 0;
				int bestZ = 0;
				int bestDistSq = INT_MAX;

				for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
				{
					for (int offsetX = -radius; offsetX <= radius; ++offsetX)
					{
						// 輪の上のマスだけ調べる(内側は前の輪で調べ終わっている)
						if (std::abs(offsetX) != radius && std::abs(offsetZ) != radius) continue;

						const int checkX = x + offsetX;
						const int checkZ = z + offsetZ;
						if (m_navGrid.IsBlocked(checkX, checkZ)) continue;

						// 同じ輪の中でも、角より辺の真ん中の方が近いので、距離で比べる
						const int distSq = offsetX * offsetX + offsetZ * offsetZ;
						if (distSq < bestDistSq)
						{
							bestDistSq = distSq;
							bestX = checkX;
							bestZ = checkZ;
						}
					}
				}

				if (bestDistSq != INT_MAX)
				{
					x = bestX;
					z = bestZ;
					return true;
				}
			}
			return false;
		}
		
		
		bool Pathfinder::HasLineOfSight(const Vector3& from, const Vector3& to) const
		{
			const float diffX = to.x - from.x;
			const float diffZ = to.z - from.z;
			const float length = std::sqrt(diffX * diffX + diffZ * diffZ);

			// 調べる回数(短い間隔で区切って、1点ずつ調べる)
			const float step = m_navGrid.GetCellSize() * LINE_CHECK_STEP_RATE;
			const int stepCount = (std::max)(1, static_cast<int>(std::ceil(length / step)));

			for (int i = 0; i <= stepCount; ++i)
			{
				const float t = static_cast<float>(i) / static_cast<float>(stepCount);
				const Vector3 point(from.x + diffX * t, from.y, from.z + diffZ * t);

				int x = 0;
				int z = 0;
				if (!m_navGrid.WorldToCell(point, x, z)) return false;
				if (m_navGrid.IsBlocked(x, z)) return false;
			}
			return true;
		}
		
		
		void Pathfinder::SmoothPath(std::vector<Vector3>& path) const
		{
			// 3点より少ない時は、飛ばせる点が無い
			if (path.size() < 3) return;

			std::vector<Vector3> result;
			result.push_back(path.front());

			// 最後に残した点から、まっすぐ行ける所まで進んで、行けなくなる手前の点を残す
			size_t anchor = 0;
			for (size_t i = 2; i < path.size(); ++i)
			{
				if (!HasLineOfSight(path[anchor], path[i]))
				{
					result.push_back(path[i - 1]);
					anchor = i - 1;
				}
			}
			result.push_back(path.back());

			path = std::move(result);
		}
	}
}