/**
 * @file ModelRender.h
 * @brief モデルレンダークラス
 */
#pragma once


namespace nsK2Engine {
	

	class ModelRender
	{
	public:
		ModelRender();
		~ModelRender();

		/**
		 * @brief モデルの初期化処理
		 * @param tkmFilePath tkmファイルパス
		 * @param fxFilePath fxファイルパス
		 */
		void Init(const char* tkmFilePath, const char* fxFilePath);

		/**
		 * @brief モデルの更新処理
		 */
		void Update();

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
		///** 環境光の構造体 */
		//AmbientLight ambientLight_;
		///** ディレクションライトの構造体 */
		//DirectionLight directionLight_;
		///** ライトの構造体 */
		//Light light_;
		///** ライトの定数バッファ */
		//LightCB lightCB_;
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
}