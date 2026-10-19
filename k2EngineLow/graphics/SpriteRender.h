/**
 * @file SpriteRender.h
 * @brief スプライトレンダークラス
 */
#pragma once


namespace nsK2EngineLow
{
	class SpriteRender : public IRender2D
	{
	public:
		SpriteRender();
		~SpriteRender() override;

		/**
		 * @brief スプライトの初期化
		 * @param fxFilePath fxファイルパス(nullptr なら sprite.fx)
		 * @param ddsFilePath ddsファイルパス
		 * @param width 幅(float)
		 * @param height 高さ(float)
		 */
		void Init(const char* fxFilePath, const char* ddsFilePath, float width, float height);
		/**
		 * @brief スプライトの初期化(sprite.fx を使う簡易版)
		 * @param ddsFilePath ddsファイルパス
		 * @param width 幅(float)
		 * @param height 高さ(float)
		 */
		void Init(const char* ddsFilePath, float width, float height);

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
		 * @brief 基底軸の取得
		 * @return 基底軸
		 */
		const Vector2& GetPivot() const
		{
			return m_pivot;
		}

		/**
		 * @brief 乗算カラーの取得
		 * @return 乗算カラー
		 */
		const Vector4& GetMulColor() const
		{
			return m_mulColor;
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

		/**
		 * @brief 基底軸の設定
		 * @param pivot 基底軸(0.0〜1.0。中心は (0.5, 0.5))
		 */
		void SetPivot(const Vector2& pivot)
		{
			m_pivot = pivot;
		}

		/**
		 * @brief 乗算カラーの設定
		 * @param color 乗算カラー(RGBA)
		 * @details アルファを変えると、半透明にできる。フェードなどに使う
		 */
		void SetMulColor(const Vector4& color)
		{
			m_mulColor = color;
		}

		// 更新処理。
		void Update();

		/**
		 * @brief 描画処理
		 * @param rc レンダリングコンテキスト
		 * @details RenderingEngine に登録するだけで、実際の描画は2Dパスで行われる
		 */
		void Draw(RenderContext& rc);

		/**
		 * @brief 2Dパスでの実際の描画
		 * @param rc レンダリングコンテキスト
		 * @details RenderingEngine の2Dパスから呼ばれる
		 */
		void OnRender2D(RenderContext& rc) override;


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
		/** 基底軸 */
		Vector2 m_pivot;
		/** 乗算カラー */
		Vector4 m_mulColor;
	};
}