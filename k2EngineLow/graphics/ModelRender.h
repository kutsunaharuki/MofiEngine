/**
 * @file ModelRender.h
 * @brief モデルレンダークラス
 */
#pragma once


namespace nsK2EngineLow
{
	class ModelRender
	{
	public:
		ModelRender();
		~ModelRender();

		/**
		 * @brief モデルの初期化処理
		 * @param tkmFilePath tkmファイルパス
		 * @param animClips アニメーションクリップ配列
		 * @param numAnimClips アニメーションクリップ配列の番号
		 * @param isShadow 影を落とすかどうか
		 * @param reShadow 影を受けるかどうか
		 * @param fxFilePath fxファイルパス
		 */
		void Init(
			const char* tkmFilePath,
			AnimationClip* animClips = nullptr,
			int numAnimClips = 0,
			const bool isShadow,
			const bool reShadow,
			const char* fxFilePath = "Assets/shader/model.fx");

		/**
		 * @brief アニメーションの再生
		 * @param clipNo Initで渡した配列の番号
		 */
		void PlayAnimation(int clipNo, float interpolateTime = 0.0f)
		{
			m_animation.Play(clipNo, interpolateTime);
		}

		/**
		 * @brief アニメーション再生速度の設定
		 * @param speed 再生速度
		 */
		void SetAnimationSpeed(float speed) { m_animationSpeed = speed; }

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

		/**
		 * @brief 位置を取得
		 * @return 位置
		 */
		const Vector3& GetPosition() const { return m_position; }
		/**
		 * @brief 回転を取得
		 * @return 回転
		 */
		const Quaternion& GetRotation() const { return m_rotation; }
		/**
		 * @brief スケールを取得
		 * @return スケール
		 */
		const Vector3& GetScale() const { return m_scale; }


	private:
		/** 骨 */
		Skeleton m_skeleton;
		/** アニメーション */
		Animation m_animation;
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
		/** 影を落とすかどうか */
		bool m_isShadowCaster;
		/** 影を受けるかどうか */
		bool m_isReceiveShadow;
		/** クリップを渡されたかどうか */
		bool m_isAnimted;
		/** アニメーション再生速度 */
		float m_animationSpeed;
	};
}