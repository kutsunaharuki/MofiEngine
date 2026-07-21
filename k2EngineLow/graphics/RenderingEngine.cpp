/**
 * @file RenderingEngine.cpp
 * @brief レンダリングエンジン
 */
#include "k2EngineLowPreCompile.h"
#include "RenderingEngine.h"


namespace nsK2EngineLow
{
	void RenderingEngine::Execute(RenderContext& rc)
	{
		// 登録されたモデルを全部描画する
		for (auto* model : m_models)
		{
			model->Draw(rc);
		}
		// マイフレームリストを空にする
		m_models.clear();
	}
}
