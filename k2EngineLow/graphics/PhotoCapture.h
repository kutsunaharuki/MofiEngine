/**
 * @file PhotoCapture.h
 * @brief 別カメラでの描画と写真の保存クラス
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief 別カメラでの描画と写真の保存クラス
	 */
	class PhotoCapture
	{
	public:
		PhotoCapture();
		~PhotoCapture();

		/**
		 * @brief 初期化
		 */
		void Init();


	public:
		/**
		 * @brief ファインダー用カメラ視点でモデルを描画する(オフスクリーンレンダリング)
		 * @param rc レンダリングコンテキスト
		 * @param models モデル配列
		 * @details 描画先は m_finderRT のため画面には表示されない
		 *          RenderingEngine::Excute()のメイン描画の後に呼ばれることを想定している
		 */
		void RenderOffscreen(RenderContext& rc, std::vector<Model*>& models);
		/**
		 * @brief ファインダーを画面に描画する
		 * @param rc レンダリングコンテキスト
		 * @details m_finderRT の内容を画面に描画する
		 */
		void RenderOnscreen(RenderContext& rc);


	public:
		/**
		 * @brief ファインダー用のカメラを取得
		 * @return ファインダー用のカメラ
		 */
		Camera& GetFinderCamera() { return m_finderCamera; }
		/**
		 * @brief ファインダーの位置を設定
		 * @param pos ファインダーの位置
		 */
		void SetFinderPosition(const Vector3& pos) { m_finderPosition = pos; }
		/**
		 * @brief ファインダーの回転を設定
		 * @param rot ファインダーの回転
		 */
		void SetFinderRotation(const Quaternion& rot) { m_finderRotation = rot; }
		/**
		 * @brief ファインダーのスケールを設定
		 * @param scale ファインダーのスケール
		 */
		void SetFinderScale(const Vector3& scale) { m_finderScale = scale; }


	private:
		/** ファインダー用のレンダリングターゲット */
		RenderTarget m_finderRT;
		/** ファインダー用のカメラ */
		Camera m_finderCamera;
		/** ファインダー用のスプライト */
		Sprite m_finderSprite;

		/** ファインダーの位置(仮の座標) */
		Vector3 m_finderPosition;
		/** ファインダーの回転(仮の回転) */
		Quaternion m_finderRotation;
		/** ファインダーのスケール(仮のスケール) */
		Vector3 m_finderScale;
	};
}

