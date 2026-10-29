/**
 * @file Pathfinder.h
 * @brief A* で、スタートからゴールまでの道順を求めるクラス
 * @details NavGrid を見て、通れるマスだけを通る道順を座標の列で返す。
 *          地図は書き換えてはいけないので、NavGrid は const 参照で待つ。
 */
#pragma once
#include <vector>


namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class NavGrid;


		/**
		 * @brief A* で、スタートからゴールまでの道順を求めるクラス
		 */
		class Pathfinder
		{
		public:
			explicit Pathfinder(const NavGrid& navGrid);
			~Pathfinder();


			/**
			 * @brief スタートからゴールまでの道順を求める
			 * @param start スタートの座標(今いる座標)
			 * @param goal ゴールの座標(行きたい座標)
			 * @param outPath 道順(通る点の座標の列。今いる位置は入らない)
			 * @return 道順が見つかれば true
			 * @details スタートやゴールが通れないマスの時は、一番近いマスに置き換える
			 */
			bool FindPath(const Vector3& start, const Vector3& goal, std::vector<Vector3>& outPath) const;


		public:
			/**
			 * @brief 道順をまっすぐにするかを設定
			 * @param isSmoothing まっすぐにするか
			 */
			void SetSmoothing(const bool isSmoothing) { m_isSmoothing = isSmoothing; }
			/**
			 * @brief 道順をまっすぐにするかを取得
			 * @return まっすぐにするなら true
			 */
			bool IsSmoothing() const { return m_isSmoothing; }


		private:
			/**
			 * @brief 隣のマスへ進めるか
			 * @param x 今のマスの横のマス番号
			 * @param z 今のマスの奥のマス番号
			 * @param dirX 進む方向の横の向き(-1, 0, 1)
			 * @param dirZ 進む方向の奥の向き(-1, 0, 1)
			 * @return 進めるなら true
			 * @details 斜めの時は、奥と横の両方が開いていないと進めない(壁の角のすり抜けを防止するため)
			 */
			bool CanMove(const int x, const int z, const int dirX, const int dirZ) const;
			/**
			 * @brief 一番近い通れるマスを探す
			 * @param x 横のマス番号(見つかったマスに置き換える)
			 * @param z 奥のマス番号(見つかったマスに置き換える)
			 * @return 見つかれば true
			 * @details 元のマスが通れるならそのまま true を返す。
			 */
			bool FindNearestOpenCell(int& x, int& z) const;
			/**
			 * @brief 2点間に通れないマスを通らずにまっすぐ行けるか
			 * @param from 始まりの座標
			 * @param to 終わりの座標
			 * @return まっすぐ行けるなら true
			 */
			bool HasLineOfSight(const Vector3& from, const Vector3& to) const;
			/**
			 * @brief 道順をまっすぐにする(見通せる点の間を飛ばす)
			 * @param path 道順(まっすぐにした結果に置き換える)
			 */
			void SmoothPath(std::vector<Vector3>& path) const;


		private:
			/** 経路探索の地図 */
			const NavGrid& m_navGrid;
			/** 道順をまっすぐにするか */
			bool m_isSmoothing;
		};
	}
}