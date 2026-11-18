/***
 * @file SampleMenu.h
 * @brief ミッションの見本の絵を出す UI
 */
#pragma once
#include "Source/UI/UIMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ミッションの見本の絵を出す UI
		 * @details 見本の絵そのものは PhotoSampleRenderer が描く。このメニューは、渡された絵を枠に出すだけ。
		 *          ファインダーを覗いていない時と、覗いている時で、出す枠(場所)を変える。
		 *          JSON : Assets/parameter/UI/Sample/Sample.json
		 */
		class SampleMenu : public UIMenuBase
		{
		public:
			SampleMenu();
			~SampleMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 見本の絵を設定する
			 * @param texture 見本の絵(見本が無い時は nullptr → 枠を消す)
			 * @details 絵は渡した側(InGameScene の PhotoSampleRenderer)が持ち続ける。
			 *          ホットリロードで UI が作り直されても、覚えている絵を渡し直す
			 */
			void SetTexture(Texture* texture);
			/**
			 * @brief ファインダーを覗いているかを設定する(出す枠が変わる)
			 * @param isFinderActive 覗いているか
			 */
			void SetIsFinderActive(const bool isFinderActive);


		private:
			/**
			 * @brief 覚えている絵と状態を、枠に反映する
			 */
			void Apply();
			/**
			 * @brief 1 つの枠に、絵と表示するかを反映する
			 * @param frame 枠
			 * @param isShow 出すか
			 */
			void ApplyFrame(UIIcon* frame, const bool isShow);


		private:
			/** ファインダーを覗いていない時の枠(ミッションの一覧の下) */
			UIIcon* m_normalFrame;
			/** ファインダーを覗いている時の枠(画面の右下) */
			UIIcon* m_finderFrame;
			/** 見本の絵(借りているだけ) */
			Texture* m_texture;
			/** ファインダーを覗いているか */
			bool m_isFinderActive;
		};
	}
}
