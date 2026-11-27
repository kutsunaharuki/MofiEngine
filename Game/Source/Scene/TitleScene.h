/**
 * @file TitleScene.h
 * @brief タイトルシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/UI/UILayout.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "IScene.h"
#include "Source/DebugTool/FlashDebugWindow.h"
#include "Source/UI/TitlePressButtonMenu/TitlePressButtonMenu.h"


namespace app
{
	namespace ui
	{
		class TitleEventMenu;
	}


	/**
	 * @brief タイトルシーン
	 * @details タイトルのイベントメニューの Presenter も兼ねる(上下で選ぶ・A で決める)
	 */
	class TitleScene : public IScene
	{
		appScene(TitleScene);


	public:
		TitleScene();
		~TitleScene() override;
		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;
		bool RequestScene(uint32_t& id, float& waitTime) override;

		/** タイトルの今の画面 */
		enum class TitleState : uint8_t
		{
			PressButton,  // 「Press A Button」を出す
			Select		  // Start / Sound / End を選ぶ
		};


	private:
		/**
		 * @brief タイトルのイベントメニューの入力を受ける(上下で選ぶ・A で決める)
		 */
		void UpdateTitleSelect();
		/**
		 * @brief 「Press A Button」の画面の入力を受ける(A で Start / Sound / End の画面に移る)
		 */
		void UpdatePressButton();


	private:
		/** インゲームへ移るかを保存する用 */
		bool m_isRequestInGame;
		/** Start を決めて、まぶしいフラッシュをしている途中か(終わったらインゲームへ移る) */
		bool m_isStartFlash;

		/** タイトルのフラッシュの調整ウィンドウ */
		debug::FlashDebugWindow m_titleFlashDebugWindow;
		/** タイトルシーンのスプライトレンダー */
		std::unique_ptr<SpriteRender> m_titleSceneSpriteRender;
		/** トランスフォーム */
		core::TransformNode m_transform;
		/** サウンドオプション(入力と音量の受け渡し。メニューの UI も持っている) */
		ui::SoundOptionController m_soundOption;
		/** タイトルイベントのUI */
		ui::UILayout m_titleEventLayout;
		/** タイトルイベントのメニュー */
		ui::TitleEventMenu* m_titleEventMenu;
		/** タイトルイベントで選んでいる項目の番号 */
		int m_titleSelectIndex;
		/** タイトルの今の画面 */
		TitleState m_titleState;
		/** 「Press A Button」の UI */
		ui::UILayout m_pressButtonLayout;
		/** 「Press A Button」のメニュー */
		ui::TitlePressButtonMenu* m_pressButtonMenu;
	};
}