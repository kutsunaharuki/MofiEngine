/**
 * @file RenderingEngine.h
 * @brief レンダリングエンジン
 */
#pragma once


namespace nsK2EngineLow
{
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
		void AddShadowCaster(Model& model) { m_shadowMapModels.push_back(&model); }
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


	private:
		/**
		 * @brief コンストラクタ
		 * @details: シングルトンパターンを採用しているため、外部からのインスタンス化を禁止するためにprivateにしている
		 */
		RenderingEngine() {}

		/** ライトカメラ */
		Camera m_lightCamera;
		/** シャドウマップ */
		RenderTarget m_shadowMap;
		/** 影を落とすモデル(毎フレーム登録) */
		std::vector<Model*> m_shadowMapModels;
		/** モデルのリスト */
		std::vector<Model*> m_models;
	};
}