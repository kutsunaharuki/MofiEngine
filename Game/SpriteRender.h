#pragma once


class SpriteRender
{
public:
	SpriteRender();
	~SpriteRender();

	// 画像の初期化。
	void Init(const char* fxFilePath, const char* ddsFilePath, int height, int width);

	/**
	 * @brief 位置の取得
	 * @return 位置
	 */
	const Vector3& GetPosition() const 
	{
		return position_;
	}
	
	/**
	 * @brief 回転の取得
	 * @return 回転
	 */
	const Quaternion& GetRotation() const 
	{ 
		return rotation_;
	}
	
	/**
	 * @brief 拡縮の取得
	 * @return 拡縮
	 */
	const Vector3& GetScale() const 
	{
		return scale_;
	}

	/**
	 * @brief 位置の設定
	 * @param pos 設定する位置
	 */
	void SetPosition(const Vector3& pos) 
	{
		position_ = pos;
	}

	/**
	 * @brief Quaternionで回転の設定
	 * @param rot 設定する回転
	 */
	void SetRotation(const Quaternion& rot) 
	{
		rotation_ = rot; 
	}

	/**
	 * @brief Vector3で拡縮の設定
	 * @param scale 設定する拡縮
	 */
	void SetScale(const Vector3& scale) 
	{
		scale_ = scale;
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
	SpriteInitData spriteInitData_;
	/** 画像の描画データ */
	Sprite sprite_;
	/** 位置 */
	Vector3 position_;
	/** 回転 */
	Quaternion rotation_;
	/** 拡縮 */
	Vector3 scale_;
};