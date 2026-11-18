/**
 * @file WorldTime.h
 * @brief 世界(悪党・客)の時間の進み方(スローモーション用)
 */
#pragma once


namespace app
{
	namespace util
	{
		/**
		 * @brief 世界(悪党・客)の時間の進み方
		 * @details 悪党・客の動き・アニメーション・タイマーは、g_gameTime の代わりにこれで時間を進める。
		 *          倍率を下げるとスローモーションになる。主人公・カメラ・UI は g_gameTime のまま(普通の速さ)
		 */
		class WorldTime
		{
		public:
			/**
			 * @brief 時間の倍率を取得
			 * @return 1.0 = 普通の速さ、0.3 = 3 割の速さ
			 */
			static float GetScale() { return s_scale; }
			/**
			 * @brief 時間の倍率を設定
			 * @param scale 1.0 = 普通の速さ
			 */
			static void SetScale(const float scale) { s_scale = scale; }
			/**
			 * @brief 世界の 1 フレームの経過時間(秒)
			 * @return 本当の経過時間 × 倍率
			 */
			static float GetDeltaTime() { return g_gameTime->GetFrameDeltaTime() * s_scale; }


		private:
			/** 時間の倍率 */
			static inline float s_scale = 1.0f;
		};
	}
}
