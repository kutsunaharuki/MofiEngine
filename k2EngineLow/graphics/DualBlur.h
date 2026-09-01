#pragma once


namespace nsK2EngineLow {
	class DualBlur
	{
	public:
		/**
		 * @brief テクスチャを渡して初期化させる
		 * @param originalTexture 元のテクスチャ
		 */
		void Init(Texture* originalTexture);

		/**
		 * @brief ぼかしを実行
		 * @param rc レンダリングコンテキスト
		 */
		void ExecuteOnGPU(RenderContext& rc);

		/**
		 * @brief ぼかし結果のテクスチャを取得
		 * @return upの1/2サイズまで戻した絵
		 */
		Texture& GetResultTexture() 
		{
			return m_upRT[NUM_UP - 1].GetRenderTargetTexture();
		}



		


	private:
		/**
		 * @brief スプライトを初期化
		 * @param sprite スプライト
		 * @param srcTexture 元のテクスチャ
		 * @param fxFilePath シェーダーファイルパス
		 * @param targetRT レンダリングターゲット
		 * @detalis : スプライト1枚の初期化は同じなためヘルパー関数にまとめる
		 */
		void InitSprite(
				Sprite& sprite
			,	Texture* srcTexture
			,	const char* fxFilePath
			,	RenderTarget& targetRT
		);
		
		/** 1/2, 1/4, 1/8, 1/16 */
		static const int NUM_DOWN = 4;
		/** 1/8, 1/4, 1/2 */
		static const int NUM_UP   = 3;

		/** 縮小の各段 */
		RenderTarget m_downRT[NUM_DOWN];
		/** 拡大の各段 */
		RenderTarget m_upRT[NUM_UP];
		/** ブラーdownスプライト */
		Sprite m_downSprites[NUM_DOWN];
		/** ブラーupスプライト */
		Sprite m_upSprites[NUM_UP];
	};
}