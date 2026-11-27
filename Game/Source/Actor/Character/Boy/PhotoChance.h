/**
 * @file PhotoChance.h
 * @brief フォトチャンス(柱からのぞいて撮れるか)を調べる
 */
#pragma once
#include <string>
#include <vector>


namespace app
{
	/** 前方宣言 */
	namespace stage
	{
		class Stage;
	}


	namespace actor
	{
		/** 前方宣言 */
		struct BoyParameter;


		/**
		 * @brief 柱のどちらからのぞくか
		 */
		enum class EnLeanSide : uint8_t
		{
			None,	// のぞかない
			Left,	// 柱の左からのぞく
			Right,	// 柱の右からのぞく
		};


		/**
		 * @brief フォトチャンス(柱からのぞいて撮れるか)を調べる
		 * @details 次の 2 つを満たす柱があれば、のぞける。
		 *          1. 主人公と悪党を結ぶ線の上にあって、主人公が柱の近く(leanCheckDistance 以内)にいる
		 *          2. 柱を画面に写した時に、柱が画面の真ん中をふさいでいて、はしが真ん中の近くにある
		 *             (右はしが右の 1/4 まで出ていなければ右から、左はしが左の 1/4 まで出ていなければ左からのぞく)
		 *          ※ のぞいて目の位置が動いても、ここでは動く前の位置で計算する(左右が行ったり来たりしないように)
		 */
		class PhotoChance : public Noncopyable
		{
		public:
			PhotoChance();
			~PhotoChance() = default;

			/**
			 * @brief ステージから柱を集める
			 * @param stage ステージ
			 * @param pillarName 柱のモデルの名前(モデルのパスにこの名前が入っている置物を柱にする)
			 * @details 画面の横の広さ(視野角)も、ここで覚える(ズームしていない時の広さで計算する)
			 */
			void Initialize(const stage::Stage& stage, const std::string& pillarName);

			/**
			 * @brief どちらからのぞくかを調べる
			 * @param boyPosition 主人公の足元の位置
			 * @param forwardXZ カメラの前の向き(XZ平面、長さ 1)
			 * @param rightXZ カメラの右の向き(XZ平面、長さ 1)
			 * @param enemyPositionList 悪党の位置の一覧
			 * @param parameter 主人公の調整パラメーター
			 * @return のぞく向き(のぞけなければ None)
			 */
			EnLeanSide Update(
				const Vector3& boyPosition,
				const Vector3& forwardXZ,
				const Vector3& rightXZ,
				const std::vector<Vector3>& enemyPositionList,
				const BoyParameter& parameter);

			/**
			 * @brief 柱の数を取得
			 * @return 柱の数(0 なら、柱の名前が合っていない)
			 */
			int GetPillarCount() const { return static_cast<int>(m_pillarList.size()); }


		private:
			/** 柱(XZ 平面で見た丸) */
			struct Pillar
			{
				/** 真ん中(y は使わない) */
				Vector3 center;
				/** 半径 */
				float radius;
			};

			/**
			 * @brief 主人公と悪党の間にあって、主人公に一番近い柱を探す
			 * @param boyPosition 主人公の足元の位置
			 * @param enemyPositionList 悪党の位置の一覧
			 * @param checkDistance 柱の表面から、これより近ければのぞける
			 * @return 柱(無ければ nullptr)
			 */
			const Pillar* FindPillarBetween(const Vector3& boyPosition, const std::vector<Vector3>& enemyPositionList, const float checkDistance) const;
			/**
			 * @brief 柱を画面に写して、どちらからのぞくかを決める
			 * @param pillar 柱
			 * @param boyPosition 主人公の足元の位置
			 * @param forwardXZ カメラの前の向き
			 * @param rightXZ カメラの右の向き
			 * @param edgeLimit 柱のはしが、真ん中からこれ(画面の半分を 1 とした長さ)より外に出ていたら、そちらからはのぞかない
			 * @return のぞく向き
			 */
			EnLeanSide CalcLeanSide(const Pillar& pillar, const Vector3& boyPosition, const Vector3& forwardXZ, const Vector3& rightXZ, const float edgeLimit) const;


		private:
			/** 柱の一覧 */
			std::vector<Pillar> m_pillarList;
			/** 画面の横の広さ(視野角の半分の tan。画面のはしが、前に 1 進んだ時の横のずれ) */
			float m_tanHalfFovX;
		};
	}
}
