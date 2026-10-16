/**
 * @file ResultScene.cpp
 * @brief リザルトシーン
 */
#include "stdafx.h"
#include "ResultScene.h"


namespace app
{
	namespace
	{
		/** リザルトシーンスプライトのファイルパス */
		constexpr const char* RESULT_SCENE_FILE_PATH = "Assets/spriteData/Scene/ResultScene.DDS";
		/** リザルトシーンスプライトの幅 */
		constexpr float RESULT_SCENE_WIDTH = 1920.0f;
		/** リザルトシーンスプライトの高さ */
		constexpr float RESULT_SCENE_HEIGHT = 1080.0f;
	}


	ResultScene::ResultScene()
	{}


	ResultScene::~ResultScene()
	{}
	
	
	bool ResultScene::Start()
	{
		// リザルトシーンのスプライトレンダーを作成
		m_resultSceneSpriteRender = std::make_unique<SpriteRender>();

		// 初期化
		m_resultSceneSpriteRender->Init(
			nullptr,
			RESULT_SCENE_FILE_PATH,
			RESULT_SCENE_HEIGHT, RESULT_SCENE_WIDTH
		);
		// 更新
		m_resultSceneSpriteRender->Update();
		return true;
	}
	
	
	void ResultScene::Update()
	{}
	
	
	void ResultScene::Render(RenderContext & rc)
	{}
	
	
	bool ResultScene::RequestScene(uint32_t & id, float& waitTime)
	{
		return false;
	}
}