#pragma once


class ModelRender
{
public:
	ModelRender();
	~ModelRender() = default;

	/**
	 * @brief 更新処理
	 */
	void Update();

	/**
	 * @brief 描画処理
	 */
	void Render(RenderContext& rc);

	/**
	 * @brief 位置を設定
	 * @param pos 位置
	 */
	void SetPosition(const Vector3& pos)
	{
		position_ = pos;
	}

	/**
	 * @brief 回転を設定
	 * @param rot 回転
	 */
	void SetRotation(const Quaternion& rot)
	{
		rotation_ = rot;
	}

	/**
	 * @brief スケールを設定
	 * @param scl スケール
	 */
	void SetScale(const Vector3& scl)
	{
		scale_ = scl;
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
	/** モデル初期化データ */
	ModelInitData modelInitData_;
	/** モデル */
	Model model_;
	/** 位置 */
	Vector3 position_;
	/** 回転 */
	Quaternion rotation_;
	/** スケール */
	Vector3 scale_;
};