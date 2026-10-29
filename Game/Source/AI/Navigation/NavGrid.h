/**
 * @file NavGrid.h
 * @brief 経路探索用の地図クラス。(通れる / 通れないマス)
 * @details ステージを一定の大きさのマスに分割し、置物があるマスを通れないマスとして持つ。
 *          Stage を読み込んだ後に Build で作る。敵(犯人)は読むだけ
 */
#pragma once
#include <cstdint>
#include <vector>


namespace app
{
	namespace stage
	{
		/** 前方宣言 */
		class Stage;
	}


	namespace ai
	{
		/**
		 * @brief 経路探索の地図を作る時の設定
		 * @details 敵の大きさに合わせて決める。値を変えたら Build し直す必要がある
		 */
		struct NavGridSetting
		{
			/** 1マスの大きさ */
			float cellSize;
			/** 敵の半径(置物をこの分だけ太らせて塞ぐ。壁にこすらないようにするため) */
			float agentRadius;
			/** 敵の高さ(これより上にあるものは、頭の上なので塞がない) */
			float agentHeight;
			/** 乗り越えられる高さ(これより低いものは、床とみなして塞がない) */
			float stepHeight;
			/** 床の高さ */
			float floorHeight;


			NavGridSetting()
				: cellSize(50.0f)
				, agentRadius(25.0f)
				, agentHeight(150.0f)
				, stepHeight(20.0f)
				, floorHeight(0.0f)
			{}
		};




		/******************************************************************/


		/**
		 * @brief 経路探索の地図
		 */
		class NavGrid
		{
		public:
			NavGrid();
			~NavGrid();

			/**
			 * @brief ステージから経路探索用の地図を作る
			 * @param stage ステージ
			 * @param setting 地図を作るための素材となる設定
			 * @details コリジョンがついているもののみ見る。重い為、毎フレーム呼ぶのはやめる。ステージを読み込んだ後に呼ぶ。
			 */
			void Build(const stage::Stage& stage, const NavGridSetting& setting = NavGridSetting());
			/**
			 * @brief マスが地図の範囲内かどうか
			 * @param x 横のマス番号
			 * @param z 奥のマス番号
			 * @return 範囲内なら true、範囲外なら false
			 */
			bool IsInside(const int x, const int z) const
			{
				return x >= 0 && x < m_width && z >= 0 && z < m_height;
			}
			/**
			 * @brief マスが通れないか
			 * @param x 横のマス番号
			 * @param z 奥のマス番号
			 * @return 通れないなら true(範囲外なら通れないとして true)
			 */
			bool IsBlocked(const int x, const int z) const
			{
				if (!IsInside(x, z)) return true;
				return m_blocked[ToIndex(x, z)] != 0;
			}
			/**
			 * @brief 座標からマス番号を求める
			 * @param pos 座標
			 * @param outX 横のマス番号
			 * @param outZ 奥のマス番号
			 * @return 地図の範囲内なら true、範囲外なら false
			 */
			bool WorldToCell(const Vector3& pos, int& outX, int& outZ) const;
			/**
			 * @brief マスの中心の座標を求める
			 * @param x 横のマス番号
			 * @param z 奥のマス番号
			 * @return マスの中心の座標(Y: 床の高さ)
			 */
			Vector3 CellToWorld(const int x, const int z) const;


		public:
			/**
			 * @brief マスの幅を取得する
			 * @return マスの幅
			 */
			int GetWidth() const { return m_width; }
			/**
			 * @brief マスの高さを取得する
			 * @return マスの高さ
			 */
			int GetHeight() const { return m_height; }
			/**
			 * @brief マスの大きさを取得する
			 * @return マスの大きさ
			 */
			float GetCellSize() const { return m_setting.cellSize; }
			/**
			 * @brief 地図を作った時の設定を取得する
			 * @return 地図を作った時の設定
			 * @details デバッグ表示で、Build と同じ条件(高さの帯・敵の半径)を使うため
			 */
			const NavGridSetting& GetSetting() const { return m_setting; }
			/**
			 * @brief 地図が出来ているか
			 * @return 出来ていれば true、出来ていなければ false
			 */
			bool IsBuilt() const { return !m_blocked.empty(); }


		private:
			/**
			 * @brief マス番号を配列の番号にする
			 * @param x 横のマス番号
			 * @param z 奥のマス番号
			 * @return 配列の番号
			 */
			int ToIndex(const int x, const int z) const { return z * m_width + x; }
			/**
			 * @brief 座標の x を横のマス番号にする(範囲外もそのまま返す)
			 * @param worldX 座標の X
			 * @return 横のマス番号
			 */
			int ToCellX(const float worldX) const;
			/**
			 * @brief 座標の z を奥のマス番号にする(範囲外もそのまま返す)
			 * @param worldZ 座標の Z
			 * @return 奥のマス番号
			 */
			int ToCellZ(const float worldZ) const;
			/**
			 * @brief 箱(XZ)に重なるマスを全部通れないようにする
			 * @param boxMin 箱の小さい方の角
			 * @param boxMax 箱の大きい方の角
			 */
			void BlockBox(const Vector3& boxMin, const Vector3& boxMax);


		private:
			/** 地図を作った時の設定 */
			NavGridSetting m_setting;
			/** マス(0,0)の角のワールド座標 */
			Vector3 m_origin;
			/** 横のマス */
			int m_width;
			/** 奥のマス */
			int m_height;
			/** 各マスが通れないか (1: 通れない、0:　通れる) */
			std::vector<uint8_t> m_blocked;
		};
	}
}