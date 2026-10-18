/***
 * @file SoundOptionMenu.h
 * @brief サウンドオプションに関するクラス
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include "Source/UI/Animation/UIAnimation.h"
#include <array>
#include <memory>


namespace app
{
	namespace ui
	{
		/**
		 * @brief サウンドオプションメニュークラス
		 * @details 上下で項目を選び、左右で音量を変える。Bボタンで閉じる。
		 *          JSON : Assets/parameter/UI/Sound/SoundOption.json
		 */
		class SoundOptionMenu : public UIMenuBase
		{
		public:
			SoundOptionMenu();
			~SoundOptionMenu() override;

			/** 更新処理 */
			void Update() override;
			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 開く
			 */
			void Open();
			/**
			 * @brief 閉じる
			 */
			void Close();
			/**
			 * @brief 開いているか
			 * @return 開いているかどうか
			 */
			bool IsOpen() const { return m_isOpen; }


		private:
			/** 入力の処理 */
			void UpdateInput();
			/** 音量と選択を、UIの見た目に反映する */
			void ApplyToUI();
			/**
			 * @brief 選んでいる項目の音量を変える
			 * @param step 変える段階(+1 / -1)
			 */
			void ChangeVolume(const int step);
			/**
			 * @brief 選んでいる項目を変える
			 * @param index 項目の番号
			 */
			void Select(const int index);
			/**
			 * @brief ゲージの上に、10等分の目盛り(区切り線)を作る
			 * @details ゲージの背景の位置と幅から計算するので、JSON でゲージを動かしても揃う
			 */
			void CreateScaleMarks();
			/**
			 * @brief 下線を、選んでいる文字の下に動かす
			 */
			void UpdateUnderline();


		private:
			/** 項目の数 */
			static constexpr int ITEM_COUNT = 4;


		private:
			/** 項目の文字のUI */
			std::array<UIBase*, ITEM_COUNT> m_textList;
			/** 項目の音量ゲージのUI */
			std::array<UIGauge*, ITEM_COUNT> m_gaugeList;
			/** 項目の音量ゲージの背景のUI(目盛りの位置を計算するのに使う) */
			std::array<UIImage*, ITEM_COUNT> m_gaugeBackList;
			/** 項目の音量の数値のUI */
			std::array<UIText*, ITEM_COUNT> m_valueTextList;
			/** 選んでいる文字の下線 */
			UIImage* m_underline;
			/** 開いたときのアニメーション */
			std::unique_ptr<UIAnimator> m_openAnimator;
			/** 点滅の経過時間 */
			float m_blinkTimer;
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 開いているか */
			bool m_isOpen;
		};
	}
}
