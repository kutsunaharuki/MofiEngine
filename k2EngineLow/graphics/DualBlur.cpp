#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	void DualBlur::Init(Texture* originalTexture)
	{
		/** RenderTargetを作る(全部 FP16。サイズは半分ずつ縮小 → また倍ずつ拡大) */
		int w = FRAME_BUFFER_W;
		int h = FRAME_BUFFER_H;

		// down専用レンダリングターゲットの初期化
		for (int i = 0; i < NUM_DOWN; i++)
		{
			w /= 2;
			h /= 2;
			m_downRT[i].Create(
				w, h,
				1, 1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_UNKNOWN
			);
		}

		// up専用レンダリングターゲットの初期化
		for (int i = 0; i < NUM_UP; i++)
		{
			w *= 2;
			h *= 2;
			m_upRT[i].Create(
				w, h,
				1, 1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_UNKNOWN
			);
		}

		/** スプライトを作る(「前の出力」を「次の入力」に) */
		Texture* src = originalTexture;
		for (int i = 0; i < NUM_DOWN; i++)
		{
			InitSprite(m_downSprites[i], src, "Assets/shader/dualBlurDown.fx", m_downRT[i]);
			src = &m_downRT[i].GetRenderTargetTexture(); // 今の段の出力が、次の段の出力になる
		}

		for (int i = 0; i < NUM_UP; i++)
		{
			InitSprite(m_upSprites[i], src, "Assets/shader/dualBlurUp.fx", m_upRT[i]);
			src = &m_upRT[i].GetRenderTargetTexture();
		}
	}
	
	
	void DualBlur::InitSprite(Sprite & sprite, Texture * srcTexture, const char* fxFilePath, RenderTarget & targetRT)
	{
		SpriteInitData initData;
		initData.m_textures[0] = srcTexture;        // 前の段の絵
		initData.m_fxFilePath = fxFilePath;			// down用かupようのシェーダー
		initData.m_width = targetRT.GetWidth();		// 出力サイズ = 描き先のRTのサイズ
 		initData.m_height = targetRT.GetHeight();
		sprite.Init(initData);
	}


	void DualBlur::ExecuteOnGPU(RenderContext& rc)
	{
		// ダウンサンプル
		for (int i = 0; i < NUM_DOWN; i++)
		{
			rc.WaitUntilToPossibleSetRenderTarget(m_downRT[i]);
			rc.SetRenderTargetAndViewport(m_downRT[i]);
			m_downSprites[i].Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_downSprites[i].Draw(rc);
			rc.WaitUntilFinishDrawingToRenderTarget(m_downRT[i]);
		}

		// アップサンプル
		for (int i = 0; i < NUM_UP; i++)
		{
			rc.WaitUntilToPossibleSetRenderTarget(m_upRT[i]);
			rc.SetRenderTargetAndViewport(m_upRT[i]);
			m_upSprites[i].Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_upSprites[i].Draw(rc);
			rc.WaitUntilFinishDrawingToRenderTarget(m_upRT[i]);
		}
	}
}
