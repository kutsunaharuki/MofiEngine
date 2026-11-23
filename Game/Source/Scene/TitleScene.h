/**
 * @file TitleScene.h
 * @brief タイトルシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/UI/UILayout.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "IScene.h"


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


	private:
		/**
		 * @brief タイトルのイベントメニューの入力を受ける(上下で選ぶ・A で決める)
		 */
		void UpdateTitleSelect();


	private:
		/** インゲームへ移るかを保存する用 */
		bool m_isRequestInGame;
		/** Start を決めて、まぶしいフラッシュをしている途中か(終わったらインゲームへ移る) */
		bool m_isStartFlash;

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
	};
}