/**
 * @file TitlePressButtonMenu.h
 * @brief タイトルの「Press A Button」 の表示のみ
 */
#pragma once
#include "Source/UI/UIMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief タイトルの「Press A Button」 の表示のみ
		 * @details MVP の View。文字を点滅させて、言われたら出したり消したりするだけ。
		 *          A の入力と、次の画面に移るのは Presenter(TitleScene)の役目
		 */
		class TitlePressButtonMenu : public UIMenuBase
		{
		public:
			TitlePressButtonMenu();
			~TitlePressButtonMenu() override;

			/** 更新処理 */
			void Update() override;
			/** 初期化処理 */
			void InitializeLogic() override;
			/**
			 * @brief 出すか消すかを設定
			 * @param isOpen 出すなら true
			 */
			void SetIsOpen(const bool isOpen);


		private:
			/** 「Press A Button」の文字 */
			UIText* m_pressText;
			/** JSON に書いた位置(ふわふわ上下にゆらすので、元を覚えておく) */
			Vector3 m_pressTextBasePosition;
			/** JSON に書いた色(点滅の時に、ここに濃さを掛ける) */
			Vector4 m_pressTextBaseColor;
			/** 点滅の時間 */
			float m_blinkTimer;
			/** 出しているか */
			bool m_isOpen;
		};
	}
}