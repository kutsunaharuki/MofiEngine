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
			/** 見える距離 */
			float viewDistance;
			/** 視野の角度 */
			float viewAngle;
			/** 目の高さ */
			float eyeHeight;
			/** 主人公のどこを見るか(足元からの高さ。胸のあたり) */
			float targetHeight;
			/** 見え続けたら気づくまでの時間(秒) */
			float noticeTime;


			VisionSetting()
				: viewDistance(800.0f)
				, viewAngle(90.0f)
				, eyeHeight(120.0f)
				, targetHeight(70.0f)
				, noticeTime(0.5f)
			{}
		};



		/**
		 * @brief ビジョン
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
			 * @param targetPos キャラクターのどこかの位置
			 * @param selfObject 自分の体(レイが当たらないように)
			 * @param targetObject 相手の体(レイが最初に当たれば見えているということになる)
			 * @return 気づいたフレームだけ true
			 */
			bool Update(
				const float deltaTime,
				const Vector3& eyePos, 
				const Vector3& forward, 
				const Vector3& targetPos,
				const btCollisionObject* selfObject,
				const btCollisionObject* targetObject
			);


		private:
			/**
			 * @brief 見えているか(距離 → 角度 → レイの順に調べる)
			 * @param eyePos 視点の位置
			 * @param forward 前
			 * @param targetPos キャラクターのどこかの位置
			 * @param selfObject 自分の体(レイに当たってはいけない)
			 * @param targetObject 相手の体(レイに当たれば見えている)
			 */
			bool CanSee(
				const Vector3& eyePos,
				const Vector3& forward, 
				const Vector3& targetPos,
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
			 * @brief 今見えているか
			 * @return 見えていたら true
			 */
			bool IsSeeing() const { return m_isSeeing; }
			/**
			 * @brief 見えている時間
			 * @return 見えている時間
			 */
			float GetSeeTimer() const { return m_seeTimer; }


		private:
			/** ビジョンの構造体 */
			VisionSetting m_setting;
			/** 見えているか */
			bool m_isSeeing;
			/** 見ている時間 */
			float m_seeTimer;
		};
	}
}

