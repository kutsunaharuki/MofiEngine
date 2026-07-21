/**
 * @file ModelRender.h
 * @brief モデルレンダークラス
 */
#pragma once


namespace nsK2Engine
{
	class ModelRender
	{
	public:
		ModelRender();
		~ModelRender();

		/**
		 * @brief モデルの初期化処理
		 * @param tkmFilePath tkmファイルパス
		 * @param fxFilePath fxファイルパス
		 * @param isShadow 影を落とすかどうか
		 * @param reShadow 影を受けるかどうか 
		 */
		void Init(const char* tkmFilePath, const char* fxFilePath, const bool isShadow, const bool reShadow);

		/**
		 * @brief モデルの更新処理
		 */
		void Update();

		/**
		 * @brief 影を落とすかどうかの設定
		 * @param isShadow 影を落とすかどうか
		 */
		void SetShadow(bool reShadow)
		{
			m_isReceiveShadow = reShadow;
		}

		/**
		 * @brief 描画処理
		 */
		void Draw(RenderContext& rc);

		/**
		 * @brief 位置を設定
		 * @param pos 位置
		 */
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
		}

		/**
		 * @brief 回転を設定
		 * @param rot 回転
		 */
		void SetRotation(const Quaternion& rot)
		{
			m_rotation = rot;
		}

		/**
		 * @brief スケールを設定
		 * @param scl スケール
		 */
		void SetScale(const Vector3& scl)
		{
			m_scale = scl;
		}

		/**
		 * @brief 位置、回転、スケールをまとめて設定
		 * @param pos 位置
		 * @param rot 回転
		 * @param scl スケール
		 */
		void SetTRS(
			const Vector3& pos,
			const Quaternion& rot,
			const Vector3& scl)
		{
			SetPosition(pos);
			SetRotation(rot);
			SetScale(scl);
		}

		
	private:
		///** 環境光の構造体 */
		//AmbientLight ambientLight_;
		///** ディレクションライトの構造体 */
		//DirectionLight directionLight_;
		///** ライトの構造体 */
		//Light light_;
		///** ライトの定数バッファ */
		//LightCB m_lightCB;
		/** 影を落とすかどうか */
		bool m_isShadow;
		/** 影を受けるかどうか */
		bool m_isReceiveShadow;
		/** モデル初期化データ */
		ModelInitData m_modelInitData;
		/** モデル */
		Model m_model;
		/** 位置 */
		Vector3 m_position;
		/** 回転 */
		Quaternion m_rotation;
		/** スケール */
		Vector3 m_scale;
	};
}