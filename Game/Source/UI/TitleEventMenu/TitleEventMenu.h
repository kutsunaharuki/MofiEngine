/**
 * @file TitleEventMenu.h
 * @brief タイトルのイベントメニュー
 * @details タイトルのの下のスタートやサウンド等の項目
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Animation/UIAnimationTypes.h"
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief タイトルのイベントメニュー
		 * @details MVP の View。項目を並べて、言われた項目を光らせるだけ。
		 *          入力(上下で選ぶ・A で決める)と、決めた後に何をするかは Presenter(TitleScene)の役目
		 */
		class TitleEventMenu : public UIMenuBase
		{
		public:
			/**
			 * @brief タイトル下の項目
			 */
			enum class TitleTextType : uint8_t
			{
				Start,  // スタート
				Sound,	// サウンド
				End,    // 終了
				Max		// 項目の最大数
			};


		public:
			TitleEventMenu();
			~TitleEventMenu() override;

			/** 更新処理(アニメーションのパラメーターの読み直しと、UI の更新) */
			void Update() override;
			/** 初期化処理 */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 選んでいる項目を設定する(色が変わって、アニメーションする)
			 * @param type 選んでいる項目
			 */
			void SetSelect(const TitleTextType type);


		private:
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 並べた文字の UIText */
			std::vector<UIText*> m_textList;
			/** アニメーションのパラメーター */
			UIAnimationParameter m_animationParameter;
		};
	}
}

