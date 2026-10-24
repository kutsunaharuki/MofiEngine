/**
 * @file ProfilerWindow.h
 * @brief CPU と GPU の処理時間を、ImGui のウィンドウに表示する
 */
#pragma once


namespace app
{
	namespace util
	{
		/**
		 * @brief 処理時間を表示するウィンドウ
		 * @details 計測はエンジンの Profiler が行う。ここは表示するだけ。
		 *          F1 キーで表示・非表示を切り替える
		 */
		class ProfilerWindow
		{
		public:
			ProfilerWindow();
			~ProfilerWindow() = default;

			/**
			 * @brief ウィンドウを表示する
			 * @details ImGui::NewFrame と ImGui::Render の間で呼ぶ
			 */
			void Draw();


		private:
			/** 全体(FPS・フレームの時間・CPU と GPU のどちらが重いか・グラフ) */
			void DrawSummary();
			/**
			 * @brief 区間ごとの表
			 * @param tableId 表の ID
			 * @param isCpu CPU の表か(false なら GPU)
			 */
			void DrawSampleTable(const char* tableId, const bool isCpu);
			/** 描いた数など */
			void DrawCounters();


		private:
			/** 表示するか */
			bool m_isOpen;
		};
	}
}
