/**
 * @file FoundMenu.h
 * @brief 悪党に見つかった時の UI(悪党の頭の上の「！」と、画面を暗くして出す「見つかってしまった...」)
 * @details MVP の View。ゲームオーバーの演出を出すだけで、いつ見つかったか・いつリザルトへ移るかは知らない。
 *          見つけた悪党を探して、頭の画面の位置を渡すのと、走り終わったら暗くするのは Presenter(InGameScene)の役目。
 *          ① ShowMark : 「！」がポンと大きく出る(頭の上についていく)
 *          ② StartGameOver : 画面がうっすら暗くなって、「見つかってしまった...」が出る(「！」は消える)
 *          ③ IsFinished : 文字が出て少し待ったら true(リザルトへ移ってよい)
 *          JSON : Assets/parameter/UI/Found/Found.json
 */
#pragma once
#include "Source/UI/UIMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 悪党に見つかった時の UI
		 */
		class FoundMenu : public UIMenuBase
		{
		public:
			FoundMenu();
			~FoundMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;
			/** 更新処理(「！」と、暗くなる動きを進める) */
			void Update() override;


		public:
			/**
			 * @brief 「！」を出す(ポンと大きく出る)
			 */
			void ShowMark();
			/**
			 * @brief 「！」を消す(悪党が主人公を見失った時)
			 */
			void HideMark();
			/**
			 * @brief 「！」を出す場所を設定する(毎フレーム呼ぶ)
			 * @param screenPosition 悪党の頭の上の、画面の位置(真ん中が原点、上が +Y)
			 * @param isVisible 画面に写っているか(カメラの後ろなら false)
			 */
			void SetMarkPosition(const Vector2& screenPosition, const bool isVisible);
			/**
			 * @brief 画面をうっすら暗くして、「見つかってしまった...」を出す
			 */
			void StartGameOver();
			/**
			 * @brief 暗くし始めたか
			 * @return 暗くし始めたなら true
			 */
			bool IsGameOverStarted() const { return m_isGameOver; }
			/**
			 * @brief 演出が全部終わったか
			 * @return 文字が出て少し待ったら true(リザルトへ移ってよい)
			 */
			bool IsFinished() const;


		private:
			/**
			 * @brief 大きさ・色・出すかを、今の時間で決める
			 */
			void ApplyLook();


		private:
			/** 画面を暗くする板 */
			UIFrame* m_backFrame;
			/** 悪党の頭の上の「！」 */
			UIIcon* m_markIcon;
			/** 「見つかってしまった...」の文字 */
			UIText* m_messageText;

			/** 「！」の、JSON に書いた大きさ(動きの基準) */
			Vector3 m_markBaseScale;
			/** 文字の、JSON に書いた大きさ(動きの基準) */
			Vector3 m_messageBaseScale;
			/** 文字の、JSON に書いた位置(動きの基準) */
			Vector3 m_messageBasePosition;

			/** 「！」を出しているか */
			bool m_isMarkShown;
			/** 「！」が画面に写っているか */
			bool m_isMarkVisible;
			/** 「！」を出してからの時間(秒) */
			float m_markTimer;
			/** 暗くし始めたか */
			bool m_isGameOver;
			/** 暗くし始めてからの時間(秒) */
			float m_gameOverTimer;
		};
	}
}
