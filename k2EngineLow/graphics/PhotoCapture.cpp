/**
 * @file PhotoCapture.cpp
 * @brief 別カメラでの描画と写真の保存クラス
 */
#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	namespace
	{
		/** ファインダーのフレームバッファの横幅 */
		constexpr int FINDER_FRAME_BUFFER_W = 640;
		/** ファインダーのフレームバッファの縦幅 */
		constexpr int FINDER_FRAME_BUFFER_H = 360;
		/** ファインダーのスプライトの横幅 */
		constexpr int FINDER_SPRITE_W = 640;
		/** ファインダーのスプライトの縦幅 */
		constexpr int FINDER_SPRITE_H = 360;
		/** ファインダーのカメラの遠平面までの距離 */
		constexpr float FINDER_CAMERA_FAR = 1000.0f;
		/** ファインダーのカメラの近平面までの距離 */
		constexpr float FINDER_CAMERA_NEAR = 1.0f;
		/** ファインダー用のカメラのデフォルト位置 */
		const Vector3 FINDER_CAMERA_DEFAULT_POSITION = { 0.0f,0.0f,120.0f };
		/** ファインダー用のカメラの注視点 */
		const Vector3 FINDER_CAMERA_TARGET = Vector3::Zero;


		/** 写真の解像度 */
		constexpr int PHOTO_W = 640;
		constexpr int PHOTO_H = 360;
		/** 画面上の写真の縮小率 */
		constexpr float PHOTO_SCALE = 0.8f;
		/** 画面端からの余白 */
		constexpr float PHOTO_MARGIN = 20.0f;
		/** 中心から端までの距離求めるための係数(全体の大きさの半分) */
		constexpr float HALF = 0.5f;
	}


	PhotoCapture::PhotoCapture()
		: m_finderPosition(Vector3::Zero)
		, m_finderRotation(Quaternion::Identity)
		, m_finderScale(Vector3::One)
		, m_photoPosition(Vector3::Zero)
		, m_photoScale(Vector3::One)
		, m_isFinderActive(false)
		, m_isShotRequested(false)
		, m_hasPhoto(false)
	{}


	PhotoCapture::~PhotoCapture()
	{}
	
	
	void PhotoCapture::Init()
	{
		// ====================================================
		// ファインダー用
		// ====================================================

		// クリアカラーは一旦赤色に
		// 赤いスプライトはファインダー
		float clearColor[4] = { 1.0f,0.0f,0.0f,1.0f };

		/** ファインダー用のレンダリングターゲット初期化 */
		m_finderRT.Create(
			FINDER_FRAME_BUFFER_W,              // 横の解像度
			FINDER_FRAME_BUFFER_H,              // 縦の解像度
			1,                                  // ミップマップレベル
			1,                                  // テクスチャ配列のサイズ
			DXGI_FORMAT_R16G16B16A16_FLOAT,     // カラーバッファ
			DXGI_FORMAT_D32_FLOAT,              // 深度ステンシルバッファ
			clearColor
		);

		// ファインダー用のスプライトを初期化(オンスクリーン)
		SpriteInitData finderSpriteInitData;
		finderSpriteInitData.m_fxFilePath  = "Assets/shader/sprite.fx";
		finderSpriteInitData.m_width       = FINDER_SPRITE_W;
		finderSpriteInitData.m_height      = FINDER_SPRITE_H;
		finderSpriteInitData.m_textures[0] = &m_finderRT.GetRenderTargetTexture();
		m_finderSprite.Init(finderSpriteInitData);

		/** ファインダー用のカメラ初期化 */
		m_finderCamera.SetFar(FINDER_CAMERA_FAR);
		m_finderCamera.SetNear(FINDER_CAMERA_NEAR);
		/** RTから取得した高さ */
		m_finderCamera.SetHeight(FINDER_SPRITE_H);
		/** RTから取得した幅 */
		m_finderCamera.SetWidth(FINDER_SPRITE_W);
		/** カメラの位置 */
		m_finderCamera.SetPosition(FINDER_CAMERA_DEFAULT_POSITION);
		/** 注視点 */
		m_finderCamera.SetTarget(FINDER_CAMERA_TARGET);
		/** カメラの更新 */
		m_finderCamera.Update();


		// ====================================================
		// 写真用
		// ====================================================

		/** 写真用のRT(レンダリングターゲット)を初期化 */
		float photoClearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
		m_photoRT.Create(
			PHOTO_W, PHOTO_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			photoClearColor
		);

		// 写真用のスプライトを初期化
		SpriteInitData photoSpriteInitData;
		photoSpriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		photoSpriteInitData.m_width = PHOTO_W;
		photoSpriteInitData.m_height = PHOTO_H;
		photoSpriteInitData.m_textures[0] = &m_photoRT.GetRenderTargetTexture();
		m_photoSprite.Init(photoSpriteInitData);


		// 写真のスケールを設定
		m_photoScale = Vector3(PHOTO_SCALE, PHOTO_SCALE, 1.0f);

		// 画面サイズをfloatに変換(フレームバッファの値がUINTのためfloat型に変換して使えるようにする)
		const float screenW = static_cast<float>(FRAME_BUFFER_W);
		const float screenH = static_cast<float>(FRAME_BUFFER_H);
		const float w = PHOTO_W * PHOTO_SCALE;
		const float h = PHOTO_H * PHOTO_SCALE;

		// 画面の中心から端までの距離を計算
		const float screenHalfW = screenW * HALF;
		const float screenHalfH = screenH * HALF;

		// 写真の中心から端までの距離を計算
		const float photoHalfW = w * HALF;
		const float photoHalfH = h * HALF;
		
		// 写真の位置
		m_photoPosition = Vector3(
			 screenHalfW - photoHalfW - PHOTO_MARGIN, // x: 右端から内側へ(右端 - 写真の半分 - 余白)
		 	-screenHalfH + photoHalfH + PHOTO_MARGIN, // y: 下端から内側へ(下端 + 写真の半分 + 余白)
			0.0f
		);
	}


	void PhotoCapture::RenderOffscreen(RenderContext& rc, std::vector<Model*>& models)
	{
		// ファインダーが有効でなければ描画をしない
		if (!m_isFinderActive) return;

		/** オフスクリーンレンダリング */
		// 書き込める状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_finderRT);
		// 描き先をファインダー用のRTに切り替え
		rc.SetRenderTargetAndViewport(m_finderRT);
		// 前のフレームの絵をクリア
		rc.ClearRenderTargetView(m_finderRT);

		// モデルを描画
		for (auto& model : models)
		{
			// ファインダー用のカメラ視点でモデルを描画
			// 第二引数にカメラを指定
			model->Draw(rc,m_finderCamera);
		}

		// 描画が終わるまで待つ
		rc.WaitUntilFinishDrawingToRenderTarget(m_finderRT);

		// 撮影リクエストがあればこの時だけ写真用のRTに描画する
		if (m_isShotRequested == true)
		{
			RenderPhoto(rc, models);
			// 撮影リクエストを切る
			m_isShotRequested = false;
			// 撮影済みフラグを立てる
			m_hasPhoto = true;
		}
	}


	void PhotoCapture::RenderOnscreen(RenderContext& rc)
	{
		// ファインダーを閉じた後でも写真が残っている場合は、描画する
		if (m_hasPhoto)
		{
			m_photoSprite.Update(m_photoPosition, Quaternion::Identity, m_photoScale);
			m_photoSprite.Draw(rc);
		}


		// ファインダーが有効でなければ描画をしない
		if (!m_isFinderActive) return;

		/** オンスクリーンレンダリング */
		// スプライトを更新
		m_finderSprite.Update(m_finderPosition, m_finderRotation, m_finderScale);
		// スプライトを描画
		m_finderSprite.Draw(rc);
	}


	void PhotoCapture::RenderPhoto(RenderContext& rc, std::vector<Model*>& models)
	{
		/** 写真用のRTに描画 */
		// 書き込める状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_photoRT);
		// 描画先を写真用のRTに切り替え
		rc.SetRenderTargetAndViewport(m_photoRT);
		// 前のフレームの絵をクリア
		rc.ClearRenderTargetView(m_photoRT);

		// モデルを描画
		for (auto& model : models)
		{
			// ファインダー用のカメラ視点でモデルを描画
			model->Draw(rc, m_finderCamera);
		}

		// 描画が終わるまで待つ → 以後このRTをテクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_photoRT);
	}
}