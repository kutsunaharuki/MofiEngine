/**
 * @file MissionMenu.h
 * @brief ミッションメニュー
 * @details 左上に、ミッションの目次(タイトル)と、ミッションごとの行を出す。
 *          行は JSON ではなく、MissionManager のミッションの数だけ、ここで作る
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <vector>


namespace app
{
	namespace mission
	{
		/** ミッションマネージャー */
		class MissionManager;
	}


	namespace ui
	{
		/**
		 * @brief ミッションメニュー
		 */
		class MissionMenu : public UIMenuBase
		{
		public:
			MissionMenu();
			~MissionMenu() override;

			/** 初期化処理 */
			void InitializeLogic() override;


		public:
			/**
			 * @brief ミッションの行を作成する関数
			 * @details 前に作った行は消してから作る
			 */
			void CreateMissionTexts();

			/**
			 * @brief ミッションの行の文と色を、今の進み具合に合わせる
			 * @details 撮影でミッションの枚数が増えた時に呼ぶ
			 */
			void UpdateMissionTexts();


		private:
			/**
			 * @brief タイトルの下線の長さと位置を、タイトルに合わせる
			 * @details 線の画像は横幅 1 にしておき、スケールの x で長さを決める
			 */
			void FitTitleLine();


		public:
			/**
			 * @brief ミッションマネージャーを設定する
			 * @param missionManager ミッションマネージャー
			 * @details 設定したら、ミッションの行を作る
			 */
			void SetMissionManager(const mission::MissionManager* missionManager);


		private:
			/** ミッションタイトルの下線 */
			UIIcon* m_mainUnderLine;
			/** ミッションタイトルテキスト(目次になる) */
			UIText* m_titleText;
			/** ミッションの行(1つのミッションに1行) */
			std::vector<UIText*> m_missionTextList;
			/** 読むだけのマネージャー */
			const mission::MissionManager* m_missionManager;
		};
	}
}
