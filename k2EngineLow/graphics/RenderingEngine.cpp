/**
 * @file RenderingEngine.cpp
 * @brief レンダリングエンジン
 */
#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	namespace
	{
		/** ライトカメラの原点からの距離 */
		constexpr float LIGHT_CAMERA_VECTOR = -1000.0f;
	}


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
		// パス2: G-Buffer パス(ディファードレンダリング)
		// モデルを、アルベド・法線・ワールド座標の3枚に同時に描く(ライティングはまだしない)
		// =============================================

		RenderTarget* gBuffers[] = { &m_gAlbedoRT, &m_gNormalRT, &m_gDepthRT };
		const int gBufferCount = static_cast<int>(sizeof(gBuffers) / sizeof(gBuffers[0]));

		rc.WaitUntilToPossibleSetRenderTargets(gBufferCount, gBuffers);
		rc.SetRenderTargetsAndViewport(gBufferCount, gBuffers);
		rc.ClearRenderTargetViews(gBufferCount, gBuffers);

		for (auto* model : m_models)
		{
			// カメラ視点で描画(renderToGBuffer.fx)
			model->Draw(rc);
		}
		// 描き終わるまで待つ → 以後テクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTargets(gBufferCount, gBuffers);

		// =============================================
		// パス3: ディファードライティングパス
		// G-Buffer とシャドウマップを読んで、ライティングと影を計算し、メインレンダーターゲットに描く
		// =============================================

		// 視点の位置を、今のカメラの位置にする(スペキュラと、DoF 用の距離の計算に使う)
		SceneLight::GetInstance().GetLightCB().light.eyePos = g_camera3D->GetPosition();

		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		// 空の色でクリアする(モデルが無いピクセルは、ライティングのシェーダーが discard するので、この色が残る)
		rc.ClearRenderTargetView(m_mainRenderTarget);

		m_deferredLightingSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
		m_deferredLightingSprite.Draw(rc);

		// 書き込める状態になるまで待つ → 以後テクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

		// PhotoCaptureがあるならオフスクリーンレンダリングを行う
		if (m_photoCapture != nullptr)
		{
			// ※前まではm_modelsのリストを渡していたが、m_modelsは毎フレーム空になるので、
			// m_finderModelsのリストを渡すように変更
			// オフスクリーンに渡すリストをファインダー用のリストに移す
			m_photoCapture->RenderOffscreen(rc, m_finderModels);
		}

		// ============================================
		// コピーパス : 加工が終わった絵を画面に出す
		// ============================================

		// 原因 : ビューポートが、ファインダー用の640x360のままになっていて戻っていない
		g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
		// ビューポートをフレームバッファのサイズに戻す
		// ※これをしないと、ビューポートが小さいままになっていて、画面の左上に小さく表示されてしまうため
		rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

		m_copyToFrameBufferSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
		m_copyToFrameBufferSprite.Draw(rc);

		// ===========================================
		// 2Dパス(スプライト・フォント・imguiはこの後 = 加工の影響を受けない)
		// ===========================================

		// 登録されたスプライト・文字を、コピーパスの後に、登録した順番で描画する
		for (auto* render2D : m_render2DList)
		{
			render2D->OnRender2D(rc);
		}

		// PhotoCaptureがあるならオンスクリーンレンダリングを行う
		if (m_photoCapture != nullptr)
		{
			m_photoCapture->RenderOnscreen(rc);
		}

		// 毎フレームリストを空にする
		m_models.clear();
		// ファインダー用のリストを空にする
		m_finderModels.clear();
		// 2Dパスのリストを空にする
		m_render2DList.clear();
	}


	RenderingEngine::RenderingEngine()
		: m_screenBlurPower(0.0f)
		, m_isEnableBloom(true)
		, m_isEnableDoF(false)
		, m_bloomThreshold(1.0f)
		, m_bloomIntensity(1.0f)
		, m_photoCapture(nullptr)
	{
		// クリアカラー
		float mainClearColor[4] = { 0.5f,0.7f,1.0f,1.0f };

		// メインレンダリングターゲットの初期化
		m_mainRenderTarget.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR フォーマット
			DXGI_FORMAT_D32_FLOAT,
			mainClearColor
		);


		// 輝度抽出のレンダリングターゲットの初期化
		m_luminanceRT.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);

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

		// 輝度抽出の後の絵
		m_bloomBlur.Init(&m_luminanceRT.GetRenderTargetTexture());
		// mainRTを直接(輝度抽出にするとぼけない)
		m_dofBlur.Init(&m_mainRenderTarget.GetRenderTargetTexture());
		

		// 加算合成スプライトの初期化
		SpriteInitData additiveBlendSpriteInitData;
		additiveBlendSpriteInitData.m_width = FRAME_BUFFER_W;
		additiveBlendSpriteInitData.m_height = FRAME_BUFFER_H;
		additiveBlendSpriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		// 1/2の解像度に縮小されたテクスチャを加算合成する
		additiveBlendSpriteInitData.m_textures[0] = &m_bloomBlur.GetResultTexture();
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

		// G-Buffer とディファードライティングの初期化
		// ※ ライティングのスプライトがシャドウマップを使うので、シャドウマップを作った後に行う
		InitDeferredRendering();
	}


	void RenderingEngine::InitDeferredRendering()
	{
		// ===============================
		// G-Buffer(モデルの情報を、画面のピクセルごとに3枚に書き分ける)
		// ※ フォーマットは ModelRender の deferredInitData.m_colorBufferFormat と必ず合わせる
		// ===============================

		// アルベド(素材の色)。深度バッファはこの1枚目が持つ(3枚同時に描くときは1枚目の深度が使われる)
		float albedoClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_gAlbedoRT.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT,
			albedoClearColor
		);

		// 法線(xyz = 0~1 に詰めた法線、w = スペキュラの強さ)
		float normalClearColor[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
		m_gNormalRT.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			normalClearColor
		);

		// ワールド座標(xyz = 座標、w = 1 ならモデルがある)
		// ※ クリア色の w を 0 にしておくと、モデルが描かれなかったピクセル(背景)を見分けられる
		float worldPosClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_gDepthRT.Create(
			FRAME_BUFFER_W, FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			worldPosClearColor
		);

		// ===============================
		// ディファードライティングのスプライト(画面全体)
		// ===============================
		SpriteInitData initData;
		initData.m_width = FRAME_BUFFER_W;
		initData.m_height = FRAME_BUFFER_H;
		initData.m_fxFilePath = "Assets/shader/deferredLighting.fx";
		// t0 ~ t2 : G-Buffer、t3 ~ t6 : シャドウマップ(シェーダーの register と順番を合わせる)
		initData.m_textures[0] = &m_gAlbedoRT.GetRenderTargetTexture();
		initData.m_textures[1] = &m_gNormalRT.GetRenderTargetTexture();
		initData.m_textures[2] = &m_gDepthRT.GetRenderTargetTexture();
		for (int i = 0; i < MAX_SHADOW; i++)
		{
			initData.m_textures[3 + i] = &m_shadowMap[i].GetRenderTargetTexture();
		}
		// b1 : ライトの定数バッファ(描画のたびに VRAM にコピーされるので、ImGui で変えた値もすぐ反映される)
		initData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		initData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		// 描き先はメインレンダーターゲット(HDR)
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		m_deferredLightingSprite.Init(initData);
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
			m_lightCamera[i].SetPosition(lightDir * LIGHT_CAMERA_VECTOR);
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