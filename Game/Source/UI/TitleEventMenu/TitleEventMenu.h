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

			/** 更新処理 */
			void Update() override;
			/** 初期化処理 */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 決定された項目を受け取る
			 * @param outPutType 結果の項目
			 * @return 決定されていたら ture
			 */
			bool ConsumeDecided(TitleTextType& outPutType);

			/**
			 * @brief 項目が選択されていたときの関数
			 * @param index 選択されている項目番号
			 */
			void Select(int index);


		public:
			/**
			 * @brief 決定しているかを取得
			 * @return 決定されていたら true(やりたいことはシーンで決める)
			 */
			bool IsDecided() const { return m_isDecided; }


		private:
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 決定されたか */
			bool m_isDecided;

			/** 選んでいる項目のタイプ */
			TitleTextType m_selectTextType;
			/** 並べた文字の UIText */
			std::vector<UIText*> m_textList;
			/** アニメーションのパラメーター */
			UIAnimationParameter m_animationParameter;
		};
	}
}

