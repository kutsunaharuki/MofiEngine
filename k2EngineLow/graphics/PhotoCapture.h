/**
 * @file PhotoCapture.h
 * @brief 別カメラでの描画と写真の保存クラス
 */
#pragma once


namespace nsK2EngineLow
{
	/** DoFのデータ */
	struct FinderDoFCB
	{
		float focusDistance; // ピントが合う距離
		float focusRange;    // ピントの合う幅
		Vector2 pad0;        // パディング(空き)


		FinderDoFCB()
			: focusDistance(1.0f)
			, focusRange(100.0f)
		{}
	};




	/****************************************************/


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
		 * @details ファインダー用のRT、カメラ、スプライを初期化している
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

		/**
		 * @brief ファインダー状態を切り替える
		 */
		void ToggleFinderActive();
		/**
		 * @brief ファインダー状態かどうか
		 * @return ファインダー状態なら true
		 */
		inline bool IsFinderActive() const { return m_isFinderActive; }
		/**
		 * @brief 撮影リクエストを送る
		 * @details ファインダー状態の時のみ有効。
		 *          実際の撮影は、RenderOffscreen() で行われる
		 */
		void Shot() { if (m_isFinderActive) m_isShotRequested = true; }
		/**
		 * @brief 撮影済みの写真があるかどうかをチェックする
		 * @return 撮影済みの写真がある場合は、true
		 */
		inline bool HasPhoto() const { return m_hasPhoto; }
		/**
		 * @brief ファインダーをズームする
		 * @param amount ズーム量(+:拡大, -:縮小)
		 */
		void FinderZoom(float amount);
		/**
		 * @brief ファインダーのズーム比率を取得
		 * @return ファインダーのズーム比率
		 */
		float GetFinderZoomRatio() const;
		/**
		 * @brief ピント距離を動かす
		 * @param amount 正:遠く、負:近く
		 */
		void FinderFocus(float amount);
		/**
		 * @brief 被写体の距離にピントが合っているかどうかを判定
		 * @param distance カメラから被写体までの距離
		 * @return  合っていれば true
		 */
		bool IsInFocus(float distance) const;


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
		inline void SetFinderPosition(const Vector3& pos) { m_finderPosition = pos; }
		/**
		 * @brief ファインダーの回転を設定
		 * @param rot ファインダーの回転
		 */
		inline void SetFinderRotation(const Quaternion& rot) { m_finderRotation = rot; }
		/**
		 * @brief ファインダーのスケールを設定
		 * @param scale ファインダーのスケール
		 */
		inline void SetFinderScale(const Vector3& scale) { m_finderScale = scale; }

		 
	public:
		/**
		* @brief ピント距離を設定
		* @param distance ピント距離
		*/
		inline void SetFocusDistance(float distance) { m_focusDistance = distance; }
		/**
		 * @brief ピント距離を取得
		 * @return ピント距離
		 */
		inline float GetFocusDistance() const { return m_focusDistance; }
		/**
		 * @brief ボケ量が最大になるまでの幅を取得
		 * @return ボケ量が最大になるまでの幅
		 */
		inline float GetFocusBlurRange() const { return m_focusBlurRange; }
		/**
		 * @brief ボケ量が最大になるまでの幅を設定
		 * @param range ボケ量が最大になるまでの幅
		 */
		inline void SetFocusBlurRange(float range) { m_focusBlurRange = range; }
		/**
		 * @brief ピントが合っているとみなす許容値を取得
		 * @return 許容値
		 */
		inline float GetFocusTolerance() const { return m_focusTolerance; }
		/**
		 * @brief ピントが合っているとみなす許容値を設定
		 * @param tolerance 許容値
		 */
		inline void SetFocusTolerance(float tolerance) { m_focusTolerance = tolerance; }


	private:
		/**
		 * @brief 写真用RTにファインダーカメラ視点で描画
		 * @param rc レンダリングコンテキスト
		 */
		void RenderPhoto(RenderContext& rc);


	private:
		/** 写真用のRT */
		RenderTarget m_photoRT;
		/** 写真用のスプライト */
		Sprite m_photoSprite;
		/** 写真スプライトの位置 */
		Vector3 m_photoPosition;
		/** 写真スプライトのスケール */
		Vector3 m_photoScale;
		/** 撮影リクエストフラグ */
		bool m_isShotRequested;
		/** 撮影済みかどうか */
		bool m_hasPhoto;


		/** ファインダー用のレンダリングターゲット */
		RenderTarget m_finderRT;
		/** ファインダー用のカメラ */
		Camera m_finderCamera;
		/** ファインダーの絵を写真用RTに写すスプライト */
		Sprite m_photoCopySprite;
		/** DOF適用後のRT(画面に表示する絵) */
		RenderTarget m_finderResultRT;
		/** ファインダー用のDOF用のスプライト */
		Sprite m_finderDofSprite;
		/** ファインダー用のブラー */
		DualBlur m_finderBlur;
		/** ファインダー用のDoFのデータ */
		FinderDoFCB m_finderDoFCB;

		/** ファインダー用のスプライト */
		Sprite m_finderSprite;
		/** ファインダーの位置(仮の座標) */
		Vector3 m_finderPosition;
		/** ファインダーの回転(仮の回転) */
		Quaternion m_finderRotation;
		/** ファインダーのスケール(仮のスケール) */
		Vector3 m_finderScale;

		/** ファインダー状態 */
		bool m_isFinderActive;
		/** 現在の画角(ラジアン) */
		float m_viewAngle;
		/** ピント距離 */
		float m_focusDistance;
		/** ボケ量が最大になるまでの幅 */
		float m_focusBlurRange;
		/** ピントが合っているとみなす許容値 */
		float m_focusTolerance;
	};
}

