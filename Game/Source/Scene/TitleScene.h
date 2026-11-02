/**
 * @file TitleScene.h
 * @brief タイトルシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/UI/UILayout.h"
#include "IScene.h"


namespace app
{
	namespace ui
	{
		class SoundOptionMenu;
		class TitleEventMenu;
	}


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
		/** インゲームへ移るかを保存する用 */
		bool m_isRequestInGame;

		/** タイトルシーンのスプライトレンダー */
		std::unique_ptr<SpriteRender> m_titleSceneSpriteRender;
		/** トランスフォーム */
		core::TransformNode m_transform;
		/** サウンドオプションのUI */
		ui::UILayout m_soundOptionLayout;
		/** サウンドオプションのメニュー(m_soundOptionLayout が持っている) */
		ui::SoundOptionMenu* m_soundOptionMenu;
		/** タイトルイベントのUI */
		ui::UILayout m_titleEventLayout;
		/** タイトルイベントのメニュー */
		ui::TitleEventMenu* m_titleEventMenu;
	};
}