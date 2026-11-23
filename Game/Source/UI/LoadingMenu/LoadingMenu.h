/**
 * @file LoadingMenu.h
 * @brief ロード画面の UI(暗くなった画面に「読み込み中...」とヒントを出す)
 * @details MVP の View。出すだけで、シーンの読み込みのことは知らない。
 *          いつ出すか・いつ読み込むかは SceneManager の役目。
 *          ※ シーンは 1 フレームに 1 段階ずつ作るので、点は段階の合間に進む(重い段階の間は止まって見える)
 *          JSON : Assets/parameter/UI/Loading/Loading.json
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <string>


namespace app
{
	namespace ui
	{
		/**
		 * @brief ロード画面の UI
		 */
		class LoadingMenu : public UIMenuBase
		{
		public:
			LoadingMenu();
			~LoadingMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;
			/** 更新処理(「読み込み中」の後ろの点を増やしていく) */
			void Update() override;


		public:
			/**
			 * @brief 出てくる動きを最初からやり直す(ロード画面を出すたびに呼ぶ)
			 */
			void Restart();


		private:
			/**
			 * @brief 今の時間で、点の数を決めて文字に入れる
			 */
			void ApplyText();


		private:
			/** 「読み込み中」の文字 */
			UIText* m_loadingText;
			/** 「読み込み中」の、JSON に書いた文字(この後ろに点を付ける) */
			std::wstring m_baseText;
			/** 出てからの時間(秒) */
			float m_timer;
		};
	}
}
