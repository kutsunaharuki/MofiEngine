/**
 * @file RenderingEngine.cpp
 * @brief レンダリングエンジン
 */
#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	void RenderingEngine::Execute(RenderContext& rc)
	{
		//=====================================================
		// パス1: シャドウマップパス
		//=====================================================
		for (int i = 0; i < MAX_SHADOW; i++)
		{
			// 書き込める状態になるまで待つ
			rc.WaitUntilToPossibleSetRenderTarget(m_shadowMap[i]);
			// 描き先をシャドウマップに切り替え
			rc.SetRenderTargetAndViewport(m_shadowMap[i]);
			// 白でクリア(Create で渡したクリア色)
			rc.ClearRenderTargetView(m_shadowMap[i]);
			
			for (auto* caster : m_shadowCasters)
			{
				caster->Draw(rc, m_lightCamera[i]);
			}
		}

		// シャドウキャスターリストをマイフレーム空にする
		m_shadowCasters.clear();

		for (int i = 0; i < MAX_SHADOW; i++)
		{
			// 描き終わるまで待つ → 以後テクスチャとして読める
			rc.WaitUntilFinishDrawingToRenderTarget(m_shadowMap[i]);
		}
		
		// 描き先を画面(フレームバッファ)に戻して、パス2のフォワードレンダリングパスに移行する
		g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
		rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

		// カメラ視点で描画する
		for (auto* model : m_models)
		{
			model->Draw(rc);
		}

		// 毎フレームリストを空にする
		m_models.clear();
	}


	void RenderingEngine::InitializeLight()
	{
		// 長いから省略
		auto& scLight = SceneLight::GetInstance();

		// シーンライトからディレクションライトの向きを取得してくる
		const Vector3 lightPos = Vector3(500.0f, 500.0f, 500.0f);
		const Vector3 targetPos = Vector3::Zero;
		Vector3 lightDir = targetPos - lightPos;
		lightDir.Normalize();
		scLight.SetDirection(lightDir);

		// ライトの逆側・上空に置く
		for (int i = 0; i < MAX_SHADOW; i++)
		{
			m_lightCamera[i].SetPosition(lightDir * -100.0f);
			// キャラのいるあたりを見る
			m_lightCamera[i].SetTarget(Vector3::Zero);
			// 真下を向くときはY以外の値を上げる(Yだと視線と平行になり行列が壊れるから)
			m_lightCamera[i].SetUp({ 1.0f,0.0f,0.0f });
			// 平行投影
			m_lightCamera[i].SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
			// 影を写す範囲の幅
			m_lightCamera[i].SetWidth(SHADOW_WIDTH);
			// 影を写す範囲の高さ(この範囲外には影はできない)
			m_lightCamera[i].SetHeight(SHADOW_HEIGHT);
			// ライトカメラの更新
			m_lightCamera[i].Update();

			scLight.SetLightProjMatrix(i, m_lightCamera[i].GetViewProjectionMatrix());
		}


		//m_lightCamera.SetPosition(lightDir * -100.0f);
		//// キャラのいるあたりを見る
		//m_lightCamera.SetTarget(Vector3::Zero);
		//// 真下を向くときはY以外の値を上げる(Yだと視線と平行になり行列が壊れるから)
		//m_lightCamera.SetUp({ 1.0f,0.0f,0.0f });
		//// 平行投影
		//m_lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
		//// 影を写す範囲の幅
		//m_lightCamera.SetWidth(SHADOW_WIDTH);
		//// 影を写す範囲の高さ(この範囲外には影はできない)
		//m_lightCamera.SetHeight(SHADOW_HEIGHT);
		//// ライトカメラの更新
		//m_lightCamera.Update();
	}
}