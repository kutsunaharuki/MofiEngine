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
		constexpr int FINDER_FRAME_BUFFER_W = 768;
		/** ファインダーのフレームバッファの縦幅 */
		constexpr int FINDER_FRAME_BUFFER_H = 432;
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

		
		// ファインダーのズーム
		/** 円周率 */
		constexpr float PI = 3.14159265f;
		/** 度からラジアンへの変換係数 */
		constexpr float DEG_TO_RAD = PI / 180.0f;
		/** ファインダーの画角(広角側。ズームなしの状態) */
		constexpr float FINDER_VIEW_ANGLE_WIDE = 60.0f * DEG_TO_RAD;
		/** ファインダーの画角(望遠側。ズームありの状態) */
		constexpr float FINDER_VIEW_ANGLE_TELE = 7.0f * DEG_TO_RAD;
		/** ファインダーのズーム速度 */
		constexpr float FINDER_ZOOM_SPEED = 0.01f;

		
		/** ピント距離の最小値(近い側) */
		constexpr float FOCUS_DISTANCE_MIN = 50.0f;
		/** ピント距離の最大値(遠い側。ファインダーカメラの遠平面まで) */
		constexpr float FOCUS_DISTANCE_MAX = 1000.0f;
		/** ピントを動かす速度 */
		constexpr float FOCUS_SPEED = 400.0f;
		/** ピント距離から距離制限でボケ量が最大になる */
		constexpr float FOCUS_BLUR_RANGE = 300.0f;
		/** ピント距離との差の許容値 */
		constexpr float FOCUS_TOLERANCE = 40.0f;
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
		, m_viewAngle(FINDER_VIEW_ANGLE_WIDE)
		, m_focusDistance(0.0f)
		, m_focusBlurRange(FOCUS_BLUR_RANGE)
		, m_focusTolerance(FOCUS_TOLERANCE)
	{}


	PhotoCapture::~PhotoCapture()
	{}
	
	
	void PhotoCapture::Init()
	{
		// ====================================================
		// ファインダー用
		// ====================================================

		// クリアカラーは一旦青色に
		// α値を1.0にしてしまうと背景が「距離(目の前)」で埋め尽くされてしまう
		float clearColor[4] = { 0.5f,0.7f,1.0f,FINDER_CAMERA_FAR };

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


		/** DOF適用後のRTを初期化(画面に表示する絵) */
		m_finderResultRT.Create(
			FINDER_FRAME_BUFFER_W, FINDER_FRAME_BUFFER_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);

		// ファインダーの絵をぼかす準備をする
		m_finderBlur.Init(&m_finderRT.GetRenderTargetTexture());


		// DOFスプライトを初期化(「元の絵」と「ぼかした絵」を合成する)
		SpriteInitData finderDofSpriteInitData;
		finderDofSpriteInitData.m_fxFilePath = "Assets/shader/dof.fx";
		finderDofSpriteInitData.m_width = FINDER_FRAME_BUFFER_W;
		finderDofSpriteInitData.m_height = FINDER_FRAME_BUFFER_H;
		finderDofSpriteInitData.m_textures[0] = &m_finderRT.GetRenderTargetTexture();
		finderDofSpriteInitData.m_textures[1] = &m_finderBlur.GetResultTexture();
		finderDofSpriteInitData.m_expandConstantBuffer = &m_finderDoFCB;
		finderDofSpriteInitData.m_expandConstantBufferSize = sizeof(FinderDoFCB);
		m_finderDofSprite.Init(finderDofSpriteInitData);


		// ファインダー用のスプライトを初期化(オンスクリーン)
		SpriteInitData finderSpriteInitData;
		finderSpriteInitData.m_fxFilePath  = "Assets/shader/sprite.fx";
		finderSpriteInitData.m_width       = FINDER_FRAME_BUFFER_W;
		finderSpriteInitData.m_height      = FINDER_FRAME_BUFFER_H;
		// ファインダーの絵は、DOF適用後のRTを使用する
		finderSpriteInitData.m_textures[0] = &m_finderResultRT.GetRenderTargetTexture();
		m_finderSprite.Init(finderSpriteInitData);

		// ファインダーの画角を広角側で初期化
		m_finderCamera.SetViewAngle(m_viewAngle);

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
		float photoClearColor[4] = { 0.5f,0.7f,1.0f,1.0f };
		m_photoRT.Create(
			PHOTO_W, PHOTO_H,
			1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			photoClearColor
		);

		// ファインダーの絵(DoF適用後)を写真用のRTに写すスプライトを初期化
		SpriteInitData photoCopyInitData;
		photoCopyInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		photoCopyInitData.m_width = PHOTO_W;
		photoCopyInitData.m_height = PHOTO_H;
		photoCopyInitData.m_textures[0] = &m_finderResultRT.GetRenderTargetTexture();
		m_photoCopySprite.Init(photoCopyInitData);


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

		//=====================================================
		// ファインダーカメラ視点でモデルを描画(ボケ前の絵・距離)
		//=====================================================


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


		//=====================================================
		// ボケ前の絵をぼかす
		//=====================================================
		

		// ぼかしを実行
		m_finderBlur.ExecuteOnGPU(rc);


		//=====================================================
		// DOF: 距離に応じて「元の絵」と「ぼかした絵」を合成
		//=====================================================


		// 現在のピント距離をDOFの定数バッファに設定
		m_finderDoFCB.focusDistance = m_focusDistance;
		// 許容値をDOFの定数バッファに設定
		m_finderDoFCB.focusRange = m_focusBlurRange;

		// 書き込める状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_finderResultRT);
		// 描き先をファインダーの適用後のRTに切り替え
		rc.SetRenderTargetAndViewport(m_finderResultRT);
		
		// スプライトを更新
		m_finderDofSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
		// スプライトを描画
		m_finderDofSprite.Draw(rc);

		// 描画が終わるまで待つ → 以後このRTをテクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_finderResultRT);


		//====================================================
		// リクエストが来たときだけ写真用のRTに描画
		//====================================================


		// 撮影リクエストがあればこの時だけ写真用のRTに描画する
		if (m_isShotRequested == true)
		{
			RenderPhoto(rc);
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


	void PhotoCapture::ToggleFinderActive()
	{
		m_isFinderActive = !m_isFinderActive;

		// 覗くたびにピントを近い側へ外した状態から始める
		if (m_isFinderActive)
		{
			m_focusDistance = FOCUS_DISTANCE_MIN;
		}
	}


	void PhotoCapture::FinderZoom(float amount)
	{
		// ファインダーが有効でなければズームしない
		if (!m_isFinderActive) return;

		// ズームインで画角を狭くする
		m_viewAngle -= amount * FINDER_ZOOM_SPEED;
		// ズームの制御(float型専用Clamp)
		m_viewAngle = nsK2EngineLow::FloatClamp::Clamp(m_viewAngle, FINDER_VIEW_ANGLE_TELE, FINDER_VIEW_ANGLE_WIDE);

		// ファインダー用のカメラの画角を更新
		m_finderCamera.SetViewAngle(m_viewAngle);
	}


	float PhotoCapture::GetFinderZoomRatio() const
	{
		return m_viewAngle / FINDER_VIEW_ANGLE_WIDE;
	}

	
	void PhotoCapture::FinderFocus(float amount)
	{
		// ファインダー状態じゃなければピントを動かさない
		if (!m_isFinderActive) return;

		// 1フレームに動かすピントの距離の量 = amount * FOCUS_SPEED * 1フレーム時間
		m_focusDistance += amount * FOCUS_SPEED * g_gameTime->GetFrameDeltaTime();
		// ピント距離の制御(float型専用Clamp)
		m_focusDistance = FloatClamp::Clamp(m_focusDistance, FOCUS_DISTANCE_MIN, FOCUS_DISTANCE_MAX);
	}


	bool PhotoCapture::IsInFocus(float distance) const
	{
		// ピント距離との差が、許容値以内ならピントが合っていると判定する
		return fabsf(m_focusDistance - distance) <= m_focusTolerance;
	}


	void PhotoCapture::RenderPhoto(RenderContext& rc)
	{
		/** 写真用のRTにファインダーの絵(DoF適用後)を写す */
		// 書き込める状態になるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_photoRT);
		// 描画先を写真用のRTに切り替え
		rc.SetRenderTargetAndViewport(m_photoRT);

		// DOF適用後のファインダーの絵のスプライトを更新
		m_photoCopySprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
		// DOF適用後のファインダーの絵のスプライトを描画
		m_photoCopySprite.Draw(rc);

		// 描画が終わるまで待つ → 以後このRTをテクスチャとして読める
		rc.WaitUntilFinishDrawingToRenderTarget(m_photoRT);
	}
}