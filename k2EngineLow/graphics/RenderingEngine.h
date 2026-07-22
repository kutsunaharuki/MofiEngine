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
		/** ライトの向きに乗算させる数 */
		constexpr float LIGHT_HEIGHT = -1000.0f;
		/** 影の幅 */
		constexpr float SHADOW_WIDTH = 2000.0f;
		/** 影の高さ */
		constexpr float SHADOW_HEIGHT = 2000.0f;
		/** カメラの向き */
		const Vector3 CAMERA_DIR = { 1.0f,0.0f,0.0f };
		/** ライトの位置 */
		const Vector3 LIGHT_POSITION{ 0.0f,600.0f,0.0f };
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
		Camera& GetLightCamera() { return m_lightCamera; }
		/**
		 * @brief シャドウマップのレンダリングターゲットとなるテクスチャを取得
		 * @return シャドウマップのレンダリングターゲットとなるテクスチャ
		 * @details: 「影を受けたい人(モデル)」向けの窓口(API)。RenderTarget をどう持っているかは隠蔽する。
		 */
		Texture& GetShadowMapTexture() { return m_shadowMap.GetRenderTargetTexture(); }

		/**
		 * @brief ライトカメラの初期化
		 */
		void InitializeLight();


	private:
		/**
		 * @brief コンストラクタ
		 * @details: シングルトンパターンを採用しているため、外部からのインスタンス化を禁止するためにprivateにしている
		 */
		RenderingEngine() 
		{
			// シャドウマップのクリアカラーを白に設定
			float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };
			// シャドウマップのレンダリングターゲットを作成
			m_shadowMap.Create(
				1024,1024,                        // 解像度
				1,1,							  // ミップマップ数、配列数(1でよい)
				DXGI_FORMAT_R8G8B8A8_UNORM,		  // カラーのフォーマット
				DXGI_FORMAT_D32_FLOAT,			  // 深度バッファのフォーマット
				clearColor						  // シャドウマップのクリアカラー
			);
		}

		/** ライトカメラ */
		Camera m_lightCamera;
		/** シャドウマップ */
		RenderTarget m_shadowMap;
		/** 影を落とすモデル(毎フレーム登録) */
		std::vector<Model*> m_shadowCasters;
		/** モデルのリスト */
		std::vector<Model*> m_models;
	};
}