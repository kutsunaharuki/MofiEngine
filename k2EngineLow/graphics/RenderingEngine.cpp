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
		
		// =============================================
		// フォワードレンダリングパス
		// =============================================
		
		// 描き先をメインレンダリングターゲットに切り替える
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		rc.ClearRenderTargetView(m_mainRenderTarget);

		// カメラ視点で描画する
		for (auto* model : m_models)
		{
			model->Draw(rc);
		}

		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
		
		// ここにポストプロセスのパスが入る


		// ============================================
		// コピーパス
		// ============================================
		
		g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
		m_copyToFrameBufferSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
		m_copyToFrameBufferSprite.Draw(rc);

		// ===========================================
		// 2Dパス(スプライト・フォント・imguiはこの後 = 加工の影響を受けない)
		// ===========================================
		
		// 毎フレームリストを空にする
		m_models.clear();
	}


	RenderingEngine::RenderingEngine()
	{
		// メインレンダリングターゲットの初期化
		m_mainRenderTarget.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR フォーマット
			DXGI_FORMAT_D32_FLOAT
		);

		// スプライトの初期化
		m_spriteInitData.m_width = FRAME_BUFFER_W;
		m_spriteInitData.m_height = FRAME_BUFFER_H;
		m_spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		m_spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		m_copyToFrameBufferSprite.Init(m_spriteInitData);

		// シャドウマップのクリアカラーを白に設定
		float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };
		for (int i = 0; i < MAX_SHADOW; i++)
		{
			// シャドウマップのレンダリングターゲットを作成
			m_shadowMap[i].Create(
				1024, 1024,                       // 解像度
				1, 1,							  // ミップマップ数、配列数(1でよい)
				DXGI_FORMAT_R32_FLOAT,			  // DXGI_FORMAT_R8G8B8A8_UNORMから変わった(float 1チャンネル)
				DXGI_FORMAT_D32_FLOAT,			  // 深度バッファのフォーマット
				clearColor						  // シャドウマップのクリアカラー
			);
		}
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
	}
}