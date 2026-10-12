/**
 * @file ResultScene.h
 * @brief リザルトシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"


namespace app
{
	class ResultScene : public IScene
	{
		appScene(ResultScene);


	public:
		ResultScene();
		~ResultScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext & rc) override;

		bool RequestScene(uint32_t& id, float& waitTime) override;
	};
}