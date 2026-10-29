/**
 * @file PathFollower.h
 * @brief 道順(座標の列)を順番に辿る
 * @details 今どの点を目指しているのかを持って、毎フレーム進む方向を返す。
 *          実際の移動はせずに方向だけをEnemyStateMachine に渡して Move で動かせる。
 */
#pragma once
#include <random>
#include <vector>


namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class NavGrid;


		/**
		 * @brief うろつくときの設定
		 * @details 目的地モードでは大きめ、警戒モードでは小さめ(または 0)にする
		 */
		struct WanderSetting
		{
			/** 揺れる角度の最大(度数)。0なら揺れない */
			float amplitudeDeg;
			/** 揺れる速さ(1秒に何回揺れるか) */
			float frequency;
			/** 揺れた先が壁かを調べる距離(これより先が通れなければ揺れを捨てる) */
			float probeDistance;


			WanderSetting()
				: amplitudeDeg(0.0f)
				, frequency(0.5f)
				, probeDistance(50.0f)
			{}
		};




		/***********************************************/


		/**
		 * @brief 道順(座標の列)を順番に辿る
		 */
		class PathFollower
		{
		public:
			PathFollower();
			~PathFollower();

			/**
			 * @brief 道順を設定
			 * @param path 道順
			 * @details 目指す点を最初に戻して、着いた・詰まったの状態もリセットする
			 *          揺れ方を敵ごとに変える、揺れの始まり(位相)をランダムに決め直す
			 */
			void SetPath(const std::vector<Vector3>& path);
			/**
			 * @brief 道順を消す(止まる)
			 */
			void Clear();
			/**
			 * @brief 更新処理
			 * @param position 今の位置
			 * @param deltaTime 1フレームの経過時間(秒)
			 * @details 毎フレーム呼んで、その後にGetDirection / GetMoveAmount を使用する
			 */
			void Update(const Vector3& position, const float deltaTime);


		public:
			/**
			 * @brief 進む方向を取得
			 * @return 進む方向(着いた時や道順が無いときは 0)
			 */
			const Vector3& GetDirection() const { return m_direction; }
			/**
			 * @brief 移動量を取得
			 * @return 移動量(着いたときや道順が無いときは 0)
			 */
			float GetMoveAmount() const { return m_moveAmount; }
			/**
			 * @brief 道順を持っているか
			 * @return 持っていたら true
			 */
			bool HasPath() const { return !m_path.empty(); }
			/**
			 * @brief 最後の点に着いたか
			 * @return 着いていたら true
			 */
			bool IsArrived() const { return m_isArrived; }
			/**
			 * @brief 詰まっているか(しばらく次の点に近づけていない)
			 * @return 詰まっていたら true
			 * @details true になったら、敵側で今の位置から道順を求め直す(敵同士がぶつかった時などの保険)
			 */
			bool IsStuck() const { return m_isStuck; }

			/**
			 * @brief 今目指している点を取得
			 * @return 今目指している点(道順がないときは 0)
			 */
			Vector3 GetCurrentTarget() const;
			/**
			 * @brief 道順を取得
			 * @return 道順
			 */
			const std::vector<Vector3>& GetPath() const { return m_path; }
			/**
			 * @brief 今目指している点の番号を取得(デバッグ表示用)
			 * @return 今目指している点の番号
			 */
			int GetIndex() const { return m_index; }


		public:
			/**
			 * @brief 地図を設定(ふらつきで壁を調べるのに使う)
			 * @param navGrid 地図(nullptr なら壁を調べない)
			 */
			void SetNavGrid(const NavGrid* navGrid) { m_navGrid = navGrid; }
			/**
			 * @brief 着いたとみなす距離を設定
			 * @param radius 着いたとみなす距離
			 * @details 小さすぎると点の周りをぐるぐる回る。1マスの大きさより小さくする
			 */
			void SetArriveRadius(const float radius) { m_arriveRadius = radius; }
			/**
			 * @brief ふらつきの設定
			 * @param setting ふらつきの設定
			 */
			void SetWanderSetting(const WanderSetting& setting) { m_wanderSetting = setting; }


		private:
			/**
			 * @brief 目指す点に近づいていたら、次の点に進める
			 * @param position 今の位置
			 * @details 一度に複数の点を通り過ぎることもあるので、近い間はくり返し進める
			 */
			void AdvanceIndex(const Vector3& position);
			/**
			 * @brief 進む方向をふらつかせる
			 * @param position 今の位置
			 * @param baseDirection 本来進む方向
			 * @return ふらつかせた方向
			 */
			Vector3 ApplyWander(const Vector3& position, const Vector3& baseDirection) const;
			/**
			 * @brief 詰まっているかを調べる
			 * @param position 今の位置
			 * @param deltaTime 1フレームの経過時間(秒)
			 */
			void UpdateStuck(const Vector3& position, const float deltaTime);
			/**
			 * @brief 詰まりの計測をやり直す
			 */
			void ResetStuck();


		private:
			/** 地図(ふらつきで壁を調べる用) */
			const NavGrid* m_navGrid;
			/** 道順 */
			std::vector<Vector3> m_path;
			/** 今目指している点の番号 */
			int m_index;

			/** 進む方向 */
			Vector3 m_direction;
			/** 移動量 */
			float m_moveAmount;
			/** 着いたとみなす距離 */
			float m_arriveRadius;
			/** 最後の点に着いたか */
			bool m_isArrived;

			/** ふらつきの設定 */
			WanderSetting m_wanderSetting;
			/** ふらつきの経過時間 */
			float m_wanderTime;
			/** ふらつきの始まり(敵ごとに揺れ方を変える) */
			float m_wanderPhase;
			/** 乱数(ふらつきの始まりを決める) */
			std::mt19937 m_random;

			/** 詰まりを調べるまでの経過時間 */
			float m_stuckCheckTimer;
			/** 前に調べた時の、目指す点までの距離 */
			float m_lastDistance;
			/** 近づけていない時間の合計 */
			float m_stuckTime;
			/** 詰まっているか */
			bool m_isStuck;
		};
	}
}