/**
 * @file RenderingEngine.h
 * @brief レンダリングエンジン
 * @details: Step2
 */
#pragma once


namespace nsK2EngineLow
{
	namespace
	{
		/** 影の幅 */
		constexpr float SHADOW_WIDTH = 800.0f;
		/** 影の高さ */
		constexpr float SHADOW_HEIGHT = 800.0f;
		/** カメラの向き */
		const Vector3 CAMERA_DIR = { 1.0f,0.0f,0.0f };
	}


	class RenderingEngine
	{
	public:
		/**
		 * @brief インスタンスを取得
		 * @return レンダリングエンジンのインスタンス
		 */
		static RenderingEngine& GetInstance()
		{
			static RenderingEngine instance;
			return instance;
		}

		/**
		 * @brief モデルをリストに追加
		 * @param model モデル
		 * @details: ModelRenderクラスのDrawから呼ばれる事を想定している
		 */
		void AddModel(Model& model) { m_models.push_back(&model); }

		/**
		 * @brief レンダリングエンジンの実行
		 * @param rc レンダリングコンテキスト
		 * @details: フレームの最後に呼ばれる事を想定している
		 * (例) RenderingEngine::Excute(rc) ← 登録された全モデルをパスの順番通りに描画する
		 *      1.シャドウマップ(ライト視点)
		 *      2.フォワードレンダリングパス(カメラ視点)
		 *      3.2Dパス(スプライト・フォント)
		 */
		void Execute(RenderContext& rc);

		/**
		 * @brief シャドウキャスターをリストに追加
		 * @param model シャドウモデル
		 */
		void AddShadowCaster(Model& model) { m_shadowCasters.push_back(&model); }
		/**
		 * @brief ライトカメラを取得
		 * @return ライトカメラ
		 */
		Camera& GetLightCamera(int index) { return m_lightCamera[index]; }
		/**
		 * @brief シャドウマップのレンダリングターゲットとなるテクスチャを取得
		 * @return シャドウマップのレンダリングターゲットとなるテクスチャ
		 * @details: 「影を受けたい人(モデル)」向けの窓口(API)。RenderTarget をどう持っているかは隠蔽する。
		 */
		Texture& GetShadowMapTexture(int index) { return m_shadowMap[index].GetRenderTargetTexture(); }

		/**
		 * @brief シャドウマップのテクスチャをクエリ
		 * @param queryFunc クエリ関数
		 */
		void QueryShadowMapTexture(std::function<void(Texture&)> queryFunc)
		{
			for (int i = 0; i < MAX_SHADOW; i++)
			{
				queryFunc(m_shadowMap[i].GetRenderTargetTexture());
			}
		}

		/**
		 * @brief ライトカメラの初期化
		 */
		void InitializeLight();

		/**
		 * @brief 最大影数を取得
		 * @return 最大影数
		 */
		int GetMaxShadowCount() const { return MAX_SHADOW; }
		/**
		 * @brief スクリーンブラーのぼかしの強さを取得
		 * @return スクリーンブラーのぼかしの強さ
		 */
		float& GetScreenBlurPower() { return m_screenBlurPower; }
		/**
		 * @brief ブルーム効果の有効化
		 * @return 有効か無効かどうか
		 */
		bool& IsEnableBloom() { return m_isEnableBloom; }
		/**
		 * @brief ブルーム効果の閾値を取得
		 * @return ブルーム効果の閾値
		 */
		float& GetBloomThreshold() { return m_bloomCB.threshold; }
		/**
		 * @brief ブルーム効果の強さを取得
		 * @return ブルーム効果の強さ
		 */
		float& GetBloomIntensity() { return m_bloomIntensity; }

		/**
		 * @brief ブルーム効果の閾値を設定
		 * @param threshold ブルーム効果の閾値
		 */
		inline void SetBloomThreshold(float threshold)
		{
			m_bloomCB.threshold = threshold;
		}
		/**
		 * @brief ブルーム効果の強さを取得
		 * @param intensity ブルーム効果の強さ
		 */
		inline void SetBloomIntensity(float intensity)
		{
			m_bloomIntensity = intensity;
			m_additiveBlendSprite.SetMulColor(Vector4(m_bloomIntensity, m_bloomIntensity, m_bloomIntensity, 1.0f));
		}

		/**
		 * @brief ブルームのデータ
		 */
		struct BloomCB
		{
			float threshold;
			Vector3 pad9;
		};

		/**
		 * @brief ブルームのデータを取得
		 * @return ブルームのデータ
		 */
		BloomCB& GetBloomCB() { return m_bloomCB; }


	private:
		/**
		 * @brief コンストラクタ
		 * @details: シングルトンパターンを採用しているため、外部からのインスタンス化を禁止するためにprivateにしている
		 */
		RenderingEngine();

		/** ガウシアンブラー */
		GaussianBlur m_screenBlur;
		float m_screenBlurPower;

		/** ブルームのデータ */
		BloomCB m_bloomCB;

		/** 加算合成用スプライト */
		Sprite m_additiveBlendSprite;

		/** 輝度抽出の結果(画面サイズ) */
		RenderTarget m_luminanceRT;
		/** 輝度抽出のスプライト */
		Sprite m_luminanceSprite;

		/** ダウンさせる要素数 */
		static const int NUM_DOWN = 4;
		/** 1/2, 1/4, 1/8, 1/16 */
		RenderTarget m_downRT[NUM_DOWN];
		/** ブラーdownスプライト */
		Sprite m_downSprites[NUM_DOWN];
		/** アップさせる要素数 */
		static const int NUM_UP = 4;
		/** 1/8, 1/4, 1/2 */
		RenderTarget m_upRT[NUM_UP];
		/** ブラーupスプライト */
		Sprite m_upSprites[NUM_UP];

		/** ボケスプライト */
		Sprite m_bokeSprite;
		/** ボケスプライトの初期化データ */
		SpriteInitData m_bokeSpriteInitData;

		/** メインレンダリングターゲット */
		RenderTarget m_mainRenderTarget;
		/** 最後に画面へ運ぶだけのスプライト */
		Sprite m_copyToFrameBufferSprite;
		/** スプライトの初期化データ */
		SpriteInitData m_spriteInitData;

		/** 最大影数 */
		static const int MAX_SHADOW = 4;
		/** ライトカメラ */
		Camera m_lightCamera[MAX_SHADOW];
		/** シャドウマップ */
		RenderTarget m_shadowMap[MAX_SHADOW];
		/** 影を落とすモデル(毎フレーム登録) */
		std::vector<Model*> m_shadowCasters;
		/** モデルのリスト */
		std::vector<Model*> m_models;

		/** ブルーム効果の有効化 */
		bool m_isEnableBloom;
		/** ブルーム効果の閾値 */
		float m_bloomThreshold;
		/** ブルーム効果の強さ */
		float m_bloomIntensity;
	};
}