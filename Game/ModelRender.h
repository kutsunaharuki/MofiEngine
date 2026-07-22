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
		void Init(const char* tkmFilePath, const bool isShadow, const bool reShadow, const char* fxFilePath = "Assets/shader/model.fx");

		/**
		 * @brief モデルの更新処理
		 */
		void Update();

		/**
		 * @brief 影を落とすかどうかの設定
		 * @param isShadow 影を落とすかどうか
		 */
		void SetShadow(bool shadowCaster)
		{
			m_isShadowCaster = shadowCaster;
		}
		/**
		 * @brief 影を受けるかどうかの設定
		 * @param receiveShadow 影を受けるかどうか
		 */
		void SetReceiveShadow(bool receiveShadow)
		{
			m_isReceiveShadow = receiveShadow;
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
		/** 影を落とすかどうか */
		bool m_isShadowCaster;
		/** 影を受けるかどうか */
		bool m_isReceiveShadow;
		/** モデル初期化データ */
		ModelInitData m_modelInitData;
		/** モデル */
		Model m_model;
		/** シャドウモデルの初期化データ */
		ModelInitData m_shadowModelInitData;
		/** シャドウモデル */
		Model m_shadowModel;
		/** 位置 */
		Vector3 m_position;
		/** 回転 */
		Quaternion m_rotation;
		/** スケール */
		Vector3 m_scale;
	};
}