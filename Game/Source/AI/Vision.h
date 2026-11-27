/**
 * @file Vision.h
 * @brief 汎用的なビジョンクラス
 */
#pragma once


namespace app
{
	namespace ai
	{
		/**
		 * @brief 視線の設定
		 */
		struct VisionSetting
		{
			/** 見える距離(悪党を中心にした円の半径) */
			float viewDistance;
			/** 正面から左右この角度(度)までは、ふつうに見える */
			float frontAngle;
			/** 正面から左右この角度(度)までは、弱く見える(横)。これより後ろ(真後ろのまわり)は見えない */
			float sideAngle;
			/** 横(frontAngle 〜 sideAngle)の強さ(0〜1) */
			float sideRate;
			/** 円を距離で区切る輪の数(内側ほど強い) */
			int ringCount;
			/** 一番外の輪の強さ(0〜1。一番内側は 1。間は等しく変わる) */
			float farRate;
			/** 目の高さ */
			float eyeHeight;
			/** 主人公のどこを見るか(足元からの高さ。胸のあたり。頭とひざは、この高さから決める) */
			float targetHeight;
			/** 見え続けたら気づくまでの時間(秒。強さ 1 の時。弱いほど長くかかる) */
			float noticeTime;


			VisionSetting()
				: viewDistance(800.0f)
				, frontAngle(85.0f)
				, sideAngle(135.0f)
				, sideRate(0.4f)
				, ringCount(3)
				, farRate(0.4f)
				, eyeHeight(120.0f)
				, targetHeight(70.0f)
				, noticeTime(0.5f)
			{}
		};



		/**
		 * @brief ビジョン
		 * @details 悪党を中心にした円を、角度(正面・横・真後ろ)と距離(輪)のマスに区切って、相手がいるマスの強さを決める。
		 *          さらに相手の体の 3 点(頭・胸・ひざ)へ線を引き、見えた点の割合を強さにかける。
		 *          強さが 0 より大きければ「見えている」。見えている時間は、強さに合わせて増える(弱いほど気づくのが遅い)
		 */
		class Vision
		{
		public:
			Vision();
			~Vision();

			
			/**
		 	 * @brief 更新処理(見えているかを調べて、見えている時間を数える)
			 * @param deltaTime 経過時間
			 * @param eyePos 視点の位置
			 * @param forward 前
			 * @param targetFootPos 相手の足元の位置
			 * @param targetHeight 相手のどこを見るか(足元からの高さ。胸のあたり。しゃがむと低くなる)
			 * @param selfObject 自分の体(レイが当たらないように)
			 * @param targetObject 相手の体(レイが最初に当たれば見えているということになる)
			 * @param noticeScale 見つかりやすくなる倍率(遠くまで見えて、早く気づく)
			 * @return 気づいたフレームだけ true
			 */
			bool Update(
				const float deltaTime,
				const Vector3& eyePos,
				const Vector3& forward,
				const Vector3& targetFootPos,
				const float targetHeight,
				const btCollisionObject* selfObject,
				const btCollisionObject* targetObject,
				const float noticeScale
			);


		private:
			/**
			 * @brief 見える強さを求める(マスの強さ × 体の見えた点の割合)
			 * @return 0(見えない)〜 1(正面の近くで、体が全部見えている)
			 */
			float CalcStrength(
				const Vector3& eyePos,
				const Vector3& forward,
				const Vector3& targetFootPos,
				const float targetHeight,
				const btCollisionObject* selfObject,
				const btCollisionObject* targetObject,
				const float noticeScale
			) const;
			/**
			 * @brief 相手の体の 3 点(頭・胸・ひざ)のうち、間に物が無く見えた点の割合
			 * @return 0(どこも見えない)〜 1(全部見える)
			 */
			float CalcVisibleRate(
				const Vector3& eyePos,
				const Vector3& targetFootPos,
				const float targetHeight,
				const btCollisionObject* selfObject,
				const btCollisionObject* targetObject
			) const;


		public:
			/**
		 	 * @brief ビジョンを設定
			 * @param setting ビジョン設定
			 */
			void SetSetting(const VisionSetting& setting) { m_setting = setting; }
			/**
			 * @brief ビジョンの設定を取得(デバッグで円を描くのに使う)
			 * @return 最後に設定された設定
			 */
			const VisionSetting& GetSetting() const { return m_setting; }
			/**
			 * @brief ある位置がいるマス(角度と距離)の強さを求める
			 * @param eyePos 視点の位置
			 * @param forward 前
			 * @param targetFootPos 相手の足元の位置
			 * @param noticeScale 見つかりやすくなる倍率(円が大きくなる)
			 * @return 0(円の外・真後ろ)〜 1(正面の一番内側の輪)
			 * @details デバッグで円のマスを描く時も、これを使う(判定と表示がずれないように)
			 */
			float CalcCellRate(const Vector3& eyePos, const Vector3& forward, const Vector3& targetFootPos, const float noticeScale) const;

			/** 体の点へ引いた線(デバッグで描く) */
			struct DebugRay
			{
				/** 線の始まり(目) */
				Vector3 from;
				/** 線の終わり(体の点) */
				Vector3 to;
				/** 間に物が無く、見えたか */
				bool isVisible;
			};
			/** 線を引く体の点の数(頭・胸・ひざ) */
			static constexpr int BODY_POINT_COUNT = 3;
			/**
			 * @brief 最後に引いた線の数(マスの外にいた時は線を引かないので 0)
			 * @return 線の数
			 */
			int GetDebugRayCount() const { return m_debugRayCount; }
			/**
			 * @brief 最後に引いた線を取得
			 * @param index 番号(0 〜 GetDebugRayCount() - 1)
			 * @return 線
			 */
			const DebugRay& GetDebugRay(const int index) const { return m_debugRayList[index]; }
			/**
			 * @brief 今見えているか
			 * @return 見えていたら true
			 */
			bool IsSeeing() const { return m_isSeeing; }
			/**
			 * @brief 見えていない状態に戻す(見えていた時間も 0 にする)
			 * @details 相手が見つからない状態の時に、Update の代わりに呼ぶ
			 */
			void Reset()
			{
				m_isSeeing = false;
				m_seeTimer = 0.0f;
				m_strength = 0.0f;
				m_debugRayCount = 0;
			}
			/**
			 * @brief 見えている時間
			 * @return 見えている時間
			 */
			float GetSeeTimer() const { return m_seeTimer; }
			/**
			 * @brief 今の見える強さ(デバッグで表示する)
			 * @return 0(見えない)〜 1
			 */
			float GetStrength() const { return m_strength; }


		private:
			/** ビジョンの構造体 */
			VisionSetting m_setting;
			/** 見えているか */
			bool m_isSeeing;
			/** 見ている時間(強さをかけて数える) */
			float m_seeTimer;
			/** 今の見える強さ */
			float m_strength;
			/** 最後に引いた線(デバッグで描く。const の計算の中で入れるので mutable) */
			mutable DebugRay m_debugRayList[BODY_POINT_COUNT];
			/** 最後に引いた線の数 */
			mutable int m_debugRayCount;
		};
	}
}

