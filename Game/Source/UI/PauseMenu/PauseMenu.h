/**
 * @file PauseMenu.h
 * @brief ポーズ画面の UI(つづける・サウンド設定・タイトルへ)
 * @details MVP の View。選んでいる項目と押した形を出すだけで、入力もゲームの時間も知らない。
 *          開く・閉じる・時間を止める・選んだ後にすることは Presenter(InGameScene)の役目。
 *          JSON : Assets/parameter/UI/Pause/Pause.json
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <array>


namespace app
{
	namespace ui
	{
		/**
		 * @brief ポーズ画面の UI
		 */
		class PauseMenu : public UIMenuBase
		{
		public:
			/**
			 * @brief 選べる項目(上から順)
			 */
			enum class SelectType : uint8_t
			{
				Resume,		// つづける
				Sound,		// サウンド設定
				Title,		// タイトルへ
				Max
			};


		public:
			PauseMenu();
			~PauseMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 選んでいる項目を設定する
			 * @param type 選んでいる項目
			 */
			void SetSelect(const SelectType type);
			/**
			 * @brief 選んでいる項目のボタンを、押した形にする
			 */
			void PressSelect();


		private:
			/**
			 * @brief 選んでいる項目を、ボタンの見た目に入れる
			 */
			void ApplySelect();


		private:
			/** 選べる項目のボタン(SelectType の順。色と大きさはボタンが変える) */
			std::array<UIButton*, static_cast<size_t>(SelectType::Max)> m_selectButtonList;
			/** 選んでいる項目 */
			SelectType m_selectType;
		};
	}
}
