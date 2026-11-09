/**
 * @file LoadTimeWindow.h
 * @brief 読み込みにかかった時間を区間ごとに計って、ImGui のウィンドウに表示する
 */
#pragma once
#include <chrono>
#include <string>
#include <vector>


namespace app
{
	namespace debug
	{
		/**
		 * @brief 読み込み時間の計測と表示
		 * @details Begin → Mark("区間の名前") を区間の終わりごとに → End の順に呼ぶ。
		 *          Mark は「前の Mark(または Begin)からの時間」をその名前で記録する。
		 *          計測はどの構成でも行い、End で出力ウィンドウにも書く(Release でも VS の出力で見られる)。
		 *          ウィンドウは Debug / Preview の時だけ表示する
		 */
		class LoadTimeWindow : public Noncopyable
		{
		public:
			LoadTimeWindow();
			~LoadTimeWindow();

			/**
			 * @brief 計測を始める(前の記録は消す)
			 * @param title 何の読み込みか(ウィンドウと出力に出す)
			 */
			void Begin(const char* title);
			/**
			 * @brief 前の Mark(または Begin)からの時間を記録する
			 * @param name 区間の名前
			 */
			void Mark(const char* name);
			/**
			 * @brief 計測を終える(合計を出して、出力ウィンドウに書く)
			 */
			void End();

			/**
			 * @brief ウィンドウを表示する
			 * @details Debug / Preview の時だけ表示する
			 */
			void Draw();


		private:
			/** 記録を文字にする(出力ウィンドウとコピー用) */
			std::string MakeReport() const;


		private:
			/**
			 * @brief 1 つの区間の記録
			 */
			struct Section
			{
				/** 区間の名前 */
				std::string name;
				/** かかった時間(秒) */
				double seconds;
			};

			/** 時計(PC の時刻を変えても戻らない) */
			using Clock = std::chrono::steady_clock;

			/** 何の読み込みか */
			std::string m_title;
			/** 区間の一覧(計った順) */
			std::vector<Section> m_sectionList;
			/** Begin した時刻 */
			Clock::time_point m_beginTime;
			/** 最後に Mark した時刻 */
			Clock::time_point m_lastTime;
			/** 合計(秒) */
			double m_totalSeconds;
			/** 計測中か(Begin から End まで) */
			bool m_isMeasuring;
		};
	}
}
