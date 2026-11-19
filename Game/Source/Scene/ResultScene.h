/**
 * @file ResultScene.h
 * @brief リザルトシーン
 */
#pragma once
#include "IScene.h"
#include "Source/UI/UILayout.h"


namespace app
{
	namespace ui
	{
		/** リザルトの UI */
		class ResultMenu;
	}


	/**
	 * @brief リザルトシーン(撮った写真のアルバムと、もう一度 / タイトルへ)
	 * @details MVP の Presenter。GameResult(Model)を読んで表示用の値を作り、ResultMenu(View)に渡す。
	 *          入力(左右で選ぶ・A で決める)もここで受ける
	 */
	class ResultScene : public IScene
	{
		appScene(ResultScene);


	public:
		ResultScene();
		~ResultScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		bool RequestScene(uint32_t& id, float& waitTime) override;


	private:
		/**
		 * @brief 入力を受けて、選んでいる項目を動かす・決める
		 */
		void UpdateSelect();


	private:
		/** リザルトの UI レイアウト */
		ui::UILayout m_resultLayout;
		/** リザルトの UI(レイアウトが持っている。借りているだけ) */
		ui::ResultMenu* m_resultMenu;
		/** 次に移るシーン(まだ決めていなければ INVALID_SCENE_ID) */
		uint32_t m_nextSceneId;
		/** 選んでいる項目(ResultMenu::SelectType の番号) */
		int m_selectIndex;
	};
}