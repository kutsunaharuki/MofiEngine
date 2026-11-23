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
			/**
			 * @brief Start を決めた時の、画面全体のまぶしいフラッシュを始める
			 * @details 一気に真っ白になって、少しの間そのまま止まる(メニューの文字も白に消える)
			 */
			void StartScreenFlash();
			/**
			 * @brief Start のフラッシュが、真っ白で止まるところまで終わったか
			 * @return 終わったら true(この後にシーンを移る)
			 */
			bool IsScreenFlashFinished() const;


		private:
			/**
			 * @brief ロゴの Flash を、決まった間かくでぴかーんと光らせる
			 * @param deltaTime 1 フレームの時間
			 */
			void UpdateLogoFlash(const float deltaTime);


		private:
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 並べた文字の UIText */
			std::vector<UIText*> m_textList;
			/** アニメーションのパラメーター */
			UIAnimationParameter m_animationParameter;

			/** ロゴの Flash */
			UIIcon* m_logoFlash;
			/** 光った時に Flash の上に重ねる、真っ白な Flash */
			UIIcon* m_logoFlashWhite;
			/** Flash のまわりの光 */
			UIIcon* m_logoGlow;
			/** 光った時のキラッ */
			UIIcon* m_sparkle;
			/** 光った時に、画面全体をうっすら白くする板 */
			UIFrame* m_screenFlash;
			/** タイトルが出てからの時間(光る時を決める) */
			float m_flashTimer;
			/** Start を決めた時のフラッシュをしているか */
			bool m_isStartFlash;
			/** Start を決めた時のフラッシュを始めてからの時間 */
			float m_startFlashTimer;
			/** JSON に書いた大きさ(光る時に少し大きくするので、元を覚えておく) */
			Vector3 m_logoFlashBaseScale;
			Vector3 m_logoGlowBaseScale;
			Vector3 m_sparkleBaseScale;
		};
	}
}

