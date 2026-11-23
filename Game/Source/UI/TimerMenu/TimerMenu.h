/**
 * @file TimerMenu.h
 * @brief 制限時間の UI(画面の上の真ん中に、のこり時間を出す)
 * @details MVP の View。渡された のこり時間 を「分:秒」で出すだけで、時間を減らすことも、時間切れの後のことも知らない。
 *          時間を減らして渡すのと、0 になった後にリザルトへ移るのは Presenter(InGameScene)の役目。
 *          のこりが少なくなると色が変わり(黄 → 赤)、もっと少なくなると点滅して、1 秒ごとに数字がドンと大きくなる。
 *          JSON : Assets/parameter/UI/Timer/Timer.json
 */
#pragma once
#include "Source/UI/UIMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 制限時間の UI
		 */
		class TimerMenu : public UIMenuBase
		{
		public:
			TimerMenu();
			~TimerMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;
			/** 更新処理(点滅と、数字が大きくなる動きを進める) */
			void Update() override;


		public:
			/**
			 * @brief のこり時間を設定する(毎フレーム呼んでよい。数字は 1 秒変わった時だけ書きかえる)
			 * @param remainTime のこり時間(秒)
			 */
			void SetRemainTime(const float remainTime);


		private:
			/**
			 * @brief のこり時間の段階
			 */
			enum class Tone : uint8_t
			{
				Normal,		// まだある
				Warning,	// 少なくなってきた(黄)
				Danger		// もうすぐ 0(赤・点滅)
			};


		private:
			/**
			 * @brief 出している秒数から、今の段階を決める
			 * @return 段階
			 */
			Tone GetTone() const;
			/**
			 * @brief 数字を書きかえる(出している秒数を「分:秒」にする)
			 */
			void ApplyText();
			/**
			 * @brief 色・点滅・大きさを、今の時間で決める
			 */
			void ApplyLook();


		private:
			/** のこり時間の枠 */
			UIFrame* m_panel;
			/** 「のこり時間」の文字 */
			UIText* m_labelText;
			/** のこり時間の数字 */
			UIText* m_timeText;

			/** 数字の、JSON に書いた大きさ(動きの基準) */
			Vector3 m_timeBaseScale;

			/** のこり時間(秒) */
			float m_remainTime;
			/** 出している秒数(のこり時間を切り上げた数。変わった時だけ数字を書きかえる) */
			int m_shownSecond;
			/** 数字が大きくなってからの時間(秒) */
			float m_pulseTimer;
			/** 点滅の時間(秒) */
			float m_blinkTimer;
		};
	}
}
