/**
 * @file TitleScene.h
 * @brief タイトルシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"


namespace app
{
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
		/** タイトルシーンのスプライトレンダー */
		std::unique_ptr<SpriteRender> m_titleSceneSpriteRender;
		/** トランスフォーム */
		core::TransformNode m_transform;
	};
}