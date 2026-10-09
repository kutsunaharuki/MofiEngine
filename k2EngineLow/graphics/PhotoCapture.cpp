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
	}


	PhotoCapture::PhotoCapture()
		: m_finderPosition(Vector3::Zero)
		, m_finderRotation(Quaternion::Identity)
		, m_finderScale(Vector3::One)
	{}


	PhotoCapture::~PhotoCapture()
	{}
	
	
	void PhotoCapture::Init()
	{
		// クリアカラーは一旦赤色に
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
	}


	void PhotoCapture::RenderOffscreen(RenderContext& rc, std::vector<Model*>& models)
	{
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
	}


	void PhotoCapture::RenderOnscreen(RenderContext& rc)
	{
		/** オンスクリーンレンダリング */
		// スプライトを更新
		m_finderSprite.Update(m_finderPosition, m_finderRotation, m_finderScale);
		// スプライトを描画
		m_finderSprite.Draw(rc);
	}
}