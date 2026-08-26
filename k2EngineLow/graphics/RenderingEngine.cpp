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
		
		// RTとして利用できる状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		// 描き先を切り替える
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		// RTをクリア
		rc.ClearRenderTargetView(m_mainRenderTarget);
		// カメラ視点で描画
		for (auto* model : m_models)
		{
			model->Draw(rc);
		}
		// 描き終わるまで待つ → 以後テクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
		

		// RTとして利用できる状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRT);
		// 描き先を切り替える
		rc.SetRenderTargetAndViewport(m_luminanceRT);
		// RTをクリア
		rc.ClearRenderTargetView(m_luminanceRT);
		// 描画
		m_luminanceSprite.Draw(rc);
		// 描き終わるまで待つ → 以後テクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_luminanceRT);

		for (int i = 0; i < NUM_DOWN; i++)
		{
			// 書き込み可能になるまで待つ
			rc.WaitUntilToPossibleSetRenderTarget(m_downRT[i]);
			// 描き先を切り替える
			rc.SetRenderTargetAndViewport(m_downRT[i]);
			m_downSprites[i].Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
			// 描画する
			m_downSprites[i].Draw(rc);
			// 描き終わるまで待つ → 以後テクスチャとして読める
			rc.WaitUntilFinishDrawingToRenderTarget(m_downRT[i]);
		}

		for (int i = 0; i < NUM_UP; i++)
		{
			// 書き込み可能になるまで待つ
			rc.WaitUntilToPossibleSetRenderTarget(m_upRT[i]);
			// 描き先を切り替える
			rc.SetRenderTargetAndViewport(m_upRT[i]);
			m_upSprites[i].Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
			// 描画
			m_upSprites[i].Draw(rc);
			// 描き終わるまで待つ → 以後テクスチャとして読める
			rc.WaitUntilFinishDrawingToRenderTarget(m_upRT[i]);
		}

		// ============================================
		// コピーパス : 加工が終わった絵を画面に出す
		// ============================================

		if (m_isEnableBloom)
		{
			rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
			rc.SetRenderTargetAndViewport(m_mainRenderTarget);

			m_additiveBlendSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_additiveBlendSprite.Draw(rc);

			rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
		}

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
		: m_screenBlurPower(0.0f)
		, m_isEnableBloom(true)
		, m_bloomThreshold(1.0f)
		, m_bloomIntensity(1.0f)
	{
		// メインレンダリングターゲットの初期化
		m_mainRenderTarget.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR フォーマット
			DXGI_FORMAT_D32_FLOAT
		);

		// 輝度抽出のレンダリングターゲットの初期化
		m_luminanceRT.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);

		int w = FRAME_BUFFER_W;
		int h = FRAME_BUFFER_H;

		for (int i = 0; i < NUM_DOWN; i++)
		{
			// >>(ビットシフト演算子) → 2のn乗で割って、nbitずらす(小数点以下を切り捨てる)
			w /= 2;
			h /= 2;
			//int w = FRAME_BUFFER_W >> (i + 1);
			//int h = FRAME_BUFFER_H >> (i + 1);
			// down専用レンダリングターゲットの初期化
			m_downRT[i].Create(
				w, h,
				1, 1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_UNKNOWN
			);
		}

		for (int i = 0; i < NUM_UP; i++)
		{
			w *= 2;
			h *= 2;
			//int w = FRAME_BUFFER_W >> (3 - i);
			//int h = FRAME_BUFFER_H >> (3 - i);
			// up専用レンダリングターゲットの初期化
			m_upRT[i].Create(
				w, h,
				1, 1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_UNKNOWN
			);
		}

		// スプライトの初期化
		m_spriteInitData.m_width = FRAME_BUFFER_W;
		m_spriteInitData.m_height = FRAME_BUFFER_H;
		m_spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		m_spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		m_copyToFrameBufferSprite.Init(m_spriteInitData);

		// 輝度抽出スプライトの初期化
		SpriteInitData luminanceSpriteInitData;
		luminanceSpriteInitData.m_width = FRAME_BUFFER_W;
		luminanceSpriteInitData.m_height = FRAME_BUFFER_H;
		luminanceSpriteInitData.m_fxFilePath = "Assets/shader/samplingLuminance.fx";
		luminanceSpriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		luminanceSpriteInitData.m_expandConstantBuffer = &m_bloomCB;
		luminanceSpriteInitData.m_expandConstantBufferSize = sizeof(BloomCB);
		m_luminanceSprite.Init(luminanceSpriteInitData);

		// 一つ前のテクスチャを取得
		Texture* prevTexture = &m_luminanceRT.GetRenderTargetTexture();

		// downスプライトの初期化
		for (int i = 0; i < NUM_DOWN; i++)
		{
			SpriteInitData downSpriteInitData;
			downSpriteInitData.m_width = FRAME_BUFFER_W >> (i + 1);
			downSpriteInitData.m_height = FRAME_BUFFER_H >> (i + 1);
			downSpriteInitData.m_fxFilePath = "Assets/shader/dualBlurDown.fx";
			downSpriteInitData.m_textures[0] = prevTexture;
			m_downSprites[i].Init(downSpriteInitData);
			
			// 次のループで使うために、prevTextureを更新
			prevTexture = &m_downRT[i].GetRenderTargetTexture();
		}

		// upスプライトの初期化
		for (int i = 0; i < NUM_UP; i++)
		{
			SpriteInitData upSpriteInitData;
			upSpriteInitData.m_width = FRAME_BUFFER_W >> (3 - i);
			upSpriteInitData.m_height = FRAME_BUFFER_H >> (3 - i);
			upSpriteInitData.m_fxFilePath = "Assets/shader/dualBlurUp.fx";
			upSpriteInitData.m_textures[0] = prevTexture;
			m_upSprites[i].Init(upSpriteInitData);

			// 一つ前のテクスチャを更新
			prevTexture = &m_upRT[i].GetRenderTargetTexture();
		}

		// 加算合成スプライトの初期化
		SpriteInitData additiveBlendSpriteInitData;
		additiveBlendSpriteInitData.m_width = FRAME_BUFFER_W;
		additiveBlendSpriteInitData.m_height = FRAME_BUFFER_H;
		additiveBlendSpriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		// 最終結果のテクスチャを使用
		additiveBlendSpriteInitData.m_textures[0] = &m_upRT[NUM_UP - 1].GetRenderTargetTexture();
		// 加算合成にする
		additiveBlendSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;
		m_additiveBlendSprite.Init(additiveBlendSpriteInitData);


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