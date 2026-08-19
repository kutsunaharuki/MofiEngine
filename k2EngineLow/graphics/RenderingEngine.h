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


	private:
		/**
		 * @brief コンストラクタ
		 * @details: シングルトンパターンを採用しているため、外部からのインスタンス化を禁止するためにprivateにしている
		 */
		RenderingEngine() 
		{
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

		std::function<void(Texture&)> m_textureFunc;

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
	};
}