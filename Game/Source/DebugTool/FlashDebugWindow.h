/**
 * @file FlashDebugWindow.h
 * @brief 2次元空間と3次元空間のフラッシュの Debug 調整ウィンドウ
 */
#pragma once


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class TitleEventMenu;
	}


	namespace debug
	{
		/**
		 * @brief 2次元空間と3次元空間専用の Debug 調整ウィンドウ
		 */
		class FlashDebugWindow : public Noncopyable
		{
		public:
			FlashDebugWindow() = default;
			~FlashDebugWindow() = default;

			/**
			 * @brief 2Dのフラッシュの演出の調整ウィンドウ(ImGui)
			 * @param titleEventMenu タイトルのイベントメニュー
			 */
			void Draw(ui::TitleEventMenu* titleEventMenu);
		};
	}
}
