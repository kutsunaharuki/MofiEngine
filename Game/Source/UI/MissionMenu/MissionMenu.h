/**
 * @file MissionMenu.h
 * @brief ミッションメニュー
 * @details MVP の View。左上に、ミッションの目次(タイトル)と、渡された行を出すだけ。
 *          ミッション(Model)は知らない。行の値を作って渡すのは Presenter(InGameScene)の役目。
 *          行は JSON ではなく、渡された数だけ、ここで作る
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <string>
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief 行の見え方(色は View が決める)
		 */
		enum class MissionRowTone : uint8_t
		{
			Progress,		// 進行中
			Achievement,	// 達成
			Failed			// 失敗
		};


		/**
		 * @brief ミッションの 1 行
		 */
		struct MissionRowData
		{
			/** 文(マークは View が付ける) */
			std::wstring text;
			/** 見え方 */
			MissionRowTone tone = MissionRowTone::Progress;
		};




		/****************************************************/


		/**
		 * @brief ミッションメニュー
		 */
		class MissionMenu : public UIMenuBase
		{
		public:
			MissionMenu();
			~MissionMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 行を設定する
			 * @param rowList 行の一覧(上から順)
			 * @details 行の数が同じなら文と色だけ変える。違えば作り直す。
			 *          ホットリロードでも出し直せるように、コピーして持つ
			 */
			void SetMissionRows(const std::vector<MissionRowData>& rowList);


		private:
			/**
			 * @brief ミッションの行を作る
			 * @details 前に作った行は消してから作る
			 */
			void CreateMissionTexts();
			/**
			 * @brief ミッションの行の文と色を、持っている行に合わせる
			 */
			void UpdateMissionTexts();
			/**
			 * @brief タイトルの下線の長さと位置を、タイトルに合わせる
			 * @details 線の画像は横幅 1 にしておき、スケールの x で長さを決める
			 */
			void FitTitleLine();


		private:
			/** ミッションタイトルの下線 */
			UIIcon* m_mainUnderLine;
			/** ミッションタイトルテキスト(目次になる) */
			UIText* m_titleText;
			/** ミッションの行の UI(1つのミッションに1行) */
			std::vector<UIText*> m_missionTextList;
			/** 出す行 */
			std::vector<MissionRowData> m_rowDataList;
		};
	}
}
