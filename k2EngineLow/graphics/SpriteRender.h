/**
 * @file SpriteRender.h
 * @brief スプライトレンダークラス
 */
#pragma once


namespace nsK2EngineLow
{
	class SpriteRender
	{
	public:
		SpriteRender();
		~SpriteRender();

		/**
		 * @brief スプライトの初期化
		 * @param fxFilePath fxファイルパス
		 * @param ddsFilePath ddsファイルパス
		 * @param height 高さ(float)
		 * @param width 幅(float)
		 */
		void Init(const char* fxFilePath, const char* ddsFilePath, float height, float width);

		/**
		 * @brief 位置の取得
		 * @return 位置
		 */
		const Vector3& GetPosition() const
		{
			return m_position;
		}

		/**
		 * @brief 回転の取得
		 * @return 回転
		 */
		const Quaternion& GetRotation() const
		{
			return m_rotation;
		}

		/**
		 * @brief 拡縮の取得
		 * @return 拡縮
		 */
		const Vector3& GetScale() const
		{
			return m_scale;
		}

		/**
		 * @brief 位置の設定
		 * @param pos 設定する位置
		 */
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
		}

		/**
		 * @brief Quaternionで回転の設定
		 * @param rot 設定する回転
		 */
		void SetRotation(const Quaternion& rot)
		{
			m_rotation = rot;
		}

		/**
		 * @brief Vector3で拡縮の設定
		 * @param scale 設定する拡縮
		 */
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
		}

		/**
		 * @brief 位置、回転、拡縮の設定
		 * @param pos 設定する位置
		 * @param rot 設定する回転
		 * @param scale 設定する拡縮
		 */
		inline void SetTRS(const Vector3& pos, const Quaternion& rot, const Vector3& scale)
		{
			SetPosition(pos);
			SetRotation(rot);
			SetScale(scale);
		}

		// 更新処理。
		void Update();

		// 描画処理
		void Draw(RenderContext& rc);


	private:
		/** 画像の初期化データ */
		SpriteInitData m_spriteInitData;
		/** 画像の描画データ */
		Sprite m_sprite;
		/** 位置 */
		Vector3 m_position;
		/** 回転 */
		Quaternion m_rotation;
		/** 拡縮 */
		Vector3 m_scale;
	};
}