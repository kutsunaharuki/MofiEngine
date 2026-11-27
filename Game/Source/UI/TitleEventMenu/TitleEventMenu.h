/**
 * @file TitleEventMenu.h
 * @brief タイトルのイベントメニュー
 * @details タイトルのの下のスタートやサウンド等の項目
 */
#pragma once
#include "Source/UI/UIMenu.h"
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

			/** 更新処理(ロゴのフラッシュと、UI の更新) */
			void Update() override;
			/** 初期化処理 */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 選んでいる項目を設定する(文字とアイコンが黄色になって、下線が出る)
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
			/**
			 * @brief Start / Sound / End と、下のボタンの案内を出すか消すかを決める
			 * @param isOpen 出すなら true(「Press A Button」の間は false)
			 */
			void SetIsItemOpen(const bool isOpen);


		private:
			/**
			 * @brief ロゴの Flash を1回だけ光らせる
			 * @param deltaTime 1 フレームの時間
			 */
			void UpdateLogoFlash(const float deltaTime);
			/**
			 * @brief 項目の文字・アイコン・下線・帯を作る(左から 文字 → アイコン の順に並べる)
			 * @param basePosition 1 つ目の項目の文字の左はしの位置
			 */
			void CreateItemList(const Vector3& basePosition);
			/**
			 * @brief 項目の色と、下線を出すかを、選んでいる項目に合わせる
			 * @param alpha 濃さ(Start を決めた時に、白に消していくのに使う。ふだんは 1)
			 */
			void ApplyItemColor(const float alpha);


		public:
			/** カメラのフラッシュの数字 */
			struct CameraFlashSetting
			{
				/** タイトルが出るまでの遅延時間(秒) */
				float firstDelay;
				/** 一番明るくなるまでの時間(秒) */
				float riseTime;
				/** 明るさが元に戻るまでの時間(秒) */
				float fallTime;
				/** 光が一番濃いところ(0~1) */
				float lightAlpha;
				/** 光始めの大きさ */
				float lightMinScale;
				/** 光が一番大きいときの大きさ */
				float lightMaxScale;
				/** 画面全体の白さ(0~1) */
				float screenAlpha;


				CameraFlashSetting()
					: firstDelay(0.5f)
					, riseTime(0.07f)
					, fallTime(0.6f)
					, lightAlpha(0.5f)
					, lightMinScale(0.5f)
					, lightMaxScale(1.2f)
					, screenAlpha(0.05f)
				{}
			};

			/**
			 * @brief DebugToolで使用する getter
			 * @return カメラIconから出るフラッシュ
			 */
			CameraFlashSetting& GetCameraFlashSetting() { return m_cameraFlashSetting; }
			/**
			 * @brief フラッシュを最初からもう一度再生
			 */
			void ReplayCameraFlash() { m_flashTimer = 0.0f; }


		private:
			/** 選んでいる項目の番号 */
			int m_selectIndex;
			/** 並べた文字の UIText */
			std::vector<UIText*> m_textList;
			/** 文字の右に出すアイコン(Start = カメラ、Sound = 音符、End = 四角) */
			std::vector<UIIcon*> m_iconList;
			/** 選んでいる項目の下に出す下線 */
			std::vector<UIFrame*> m_underlineList;
			/** 項目を見やすくする、うすい黒い帯 */
			UIFrame* m_itemBand;

			/** カメラIconから出るフラッシュの数値情報 */
			CameraFlashSetting m_cameraFlashSetting;

			/** カメラのフラッシュの光 */
			UIIcon* m_cameraLight;
			/** フラッシュの位置 */
			Vector3 m_cameraLightBaseScale;

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
			/** キラッの JSON に書いた大きさ(光る時に大きさを変えるので、元を覚えておく) */
			Vector3 m_sparkleBaseScale;
			/** Start / Sound / End を出しているか */
			bool m_isItemOpen;
		};
	}
}

