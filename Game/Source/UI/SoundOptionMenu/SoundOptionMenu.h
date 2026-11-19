/***
 * @file SoundOptionMenu.h
 * @brief サウンドオプションに関するクラス
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include "Source/UI/UILayout.h"
#include "Source/UI/Animation/UIAnimation.h"
#include <array>
#include <memory>


namespace app
{
	namespace ui
	{
		/**
		 * @brief サウンドオプションメニュークラス
		 * @details MVP の View。言われた音量と選んでいる項目を出すだけで、SoundManager(Model)も入力も知らない。
		 *          入力を受けて音量を変えるのは SoundOptionController の役目。
		 *          JSON : Assets/parameter/UI/Sound/SoundOption.json
		 */
		class SoundOptionMenu : public UIMenuBase
		{
		public:
			/** 項目(上から順番) */
			enum class ItemType : uint8_t
			{
				Master,	// 全体の音量
				Voice,	// ボイス
				BGM,	// BGM
				SE,		// 効果音
				Max
			};

			/** 項目の数 */
			static constexpr int ITEM_COUNT = static_cast<int>(ItemType::Max);
			/** 音量の段階の数(0〜10。ゲージの目盛りもこの数で区切る) */
			static constexpr int VOLUME_STEP_COUNT = 10;


		public:
			SoundOptionMenu();
			~SoundOptionMenu() override;

			/** 更新処理(開いている時だけ、点滅とアニメーション) */
			void Update() override;
			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 出す / 消す を設定する
			 * @param isOpen 出すか(出した時は、開くアニメーションをする)
			 */
			void SetIsOpen(const bool isOpen);
			/**
			 * @brief 選んでいる項目を設定する(点滅と下線が移る)
			 * @param type 選んでいる項目
			 */
			void SetSelect(const ItemType type);
			/**
			 * @brief 項目の音量を設定する
			 * @param type 項目
			 * @param volumeStep 音量の段階(0〜VOLUME_STEP_COUNT)
			 */
			void SetVolumeStep(const ItemType type, const int volumeStep);


		private:
			/** 音量と選択を、UIの見た目に反映する */
			void ApplyToUI();
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
			/** 項目の文字のUI */
			std::array<UIBase*, ITEM_COUNT> m_textList;
			/** 項目の音量ゲージのUI */
			std::array<UIGauge*, ITEM_COUNT> m_gaugeList;
			/** 項目の音量ゲージの背景のUI(目盛りの位置を計算するのに使う) */
			std::array<UIImage*, ITEM_COUNT> m_gaugeBackList;
			/** 項目の音量の数値のUI */
			std::array<UIText*, ITEM_COUNT> m_valueTextList;
			/** 項目の音量の段階(ホットリロードでも出し直せるように持つ) */
			std::array<int, ITEM_COUNT> m_volumeStepList;
			/** 選んでいる文字の下線 */
			UIImage* m_underline;
			/** 開いたときのアニメーション */
			std::unique_ptr<UIAnimator> m_openAnimator;
			/** 点滅の経過時間 */
			float m_blinkTimer;
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 出しているか */
			bool m_isOpen;
		};




		/****************************************************/


		/**
		 * @brief サウンドオプションの入力と音量の受け渡し
		 * @details 入力を受けて SoundManager(Model)の音量を変え、SoundOptionMenu(View)に出す値を渡す。
		 *          上下で項目を選び、左右で音量を変える。A で試し聞き、B で閉じる。
		 *          タイトルとインゲームの両方で使うので、シーンではなくここに置く
		 */
		class SoundOptionController : public Noncopyable
		{
		public:
			SoundOptionController();
			~SoundOptionController();

			/**
			 * @brief 初期化(View を作る。最初は閉じている)
			 * @param layoutPath サウンドオプションのレイアウトの JSON のパス
			 */
			void Initialize(const char* layoutPath);
			/**
			 * @brief 更新処理(開いている時だけ入力を受ける)
			 */
			void Update();
			/**
			 * @brief 描画処理(閉じている時は、View の中で何も描画されない)
			 * @param rc レンダリングコンテキスト
			 */
			void Render(RenderContext& rc);


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
			 * @return 開いていれば true
			 */
			bool IsOpen() const { return m_isOpen; }


		private:
			/** 入力の処理 */
			void UpdateInput();
			/**
			 * @brief 選んでいる項目を変える
			 * @param index 項目の番号
			 */
			void Select(const int index);
			/**
			 * @brief 選んでいる項目の音量を変える
			 * @param step 変える段階(+1 / -1)
			 */
			void ChangeVolume(const int step);
			/**
			 * @brief 全部の項目の音量を、View に渡す
			 */
			void ApplyVolumesToView();


		private:
			/** サウンドオプションの UI レイアウト */
			UILayout m_layout;
			/** サウンドオプションの View(m_layout が持っている。借りているだけ) */
			SoundOptionMenu* m_view;
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 開いているか */
			bool m_isOpen;
		};
	}
}
