/**
 * @file SceneLight.h
 * @brief シーンライトクラス
 */
#pragma once


namespace nsK2EngineLow 
{
	class SceneLight
	{
	public:
		/**
		 * @brief 環境光の構造体
		 */
		struct AmbientLight
		{
			Vector3 ambient; // 環境光の色
			float pad0;		 // パディング(空けるだけ)


			AmbientLight()
				: ambient(Vector3(1.0f, 1.0f, 1.0f))
				, pad0()
			{}
		};


		/**
		 * @brief ディレクションライトの構造体
		 */
		struct DirectionLight
		{
			Vector3 direction;				// ライトの方向
			float pad0;						// パディング(空けるだけ)
			Vector3 color;					// ライトの色
			float pad1;						// パディング(空けるだけ)


			DirectionLight()
				: direction(Vector3(1.0f, -0.5f, 1.0f))
				, pad0()
				, color(Vector3(1.0f, 0.35f, 0.55f))
				, pad1()
			{}
		};


		/**
		 * @brief ライトの構造体
		 */
		struct Light
		{
			Vector3 eyePos;		// 視点の位置
			float specPower;	// スぺキュラの絞り
			float reflectPower;	// 反射の強さ
			Vector3 pad3;		// パディング(空けるだけ)


			Light()
				: eyePos(g_camera3D->GetPosition())
				, specPower(1.0f)
				, reflectPower(0.1f)
			{}
		};


		/**
		 * @brief シャドウパラメータの構造体
		 */
		struct ShadowParam
		{
			float shadowBias;               // シャドウバイアス
			Vector3 pad4;                   // パディング(空けるだけ)
			float shadowBiasMin;            // シャドウバイアスの最小値
			Vector3 pad5;                   // パディング(空けるだけ)

			ShadowParam()
				: shadowBias(0.005f)
				, shadowBiasMin(0.0001f)
			{}
		};


		/**
		 * @brief ポイントライトの構造体
		 */
		struct PTLight
		{
			Vector3 ptPosition;            // ポイントライトの位置
			float ptRange;				   // 影響範囲(距離で明るさが0になる)
			Vector3 ptColor;			   // 色
			float pad6;


			PTLight()
				: ptPosition(Vector3(0.0f, 0.0f, 0.0f))
				, ptRange(10.0f)
				, ptColor(Vector3(1.0f, 1.0f, 1.0f))
			{}
		};


		/**
		 * @brief ライトの定数バッファの構造体
		 * @details: これまで入れた物をまとめて定数バッファに入れるための構造体
		 */
		struct LightCB
		{
			DirectionLight directionLight;		   // ディレクションライト
			AmbientLight ambientLight;			   // 環境光
			Light light;						   // ライト
			Matrix mLVP;						   // ライトビュープロジェクション行列
			ShadowParam shadowParam;			   // シャドウパラメータ
			
			static const int MAX_POINT_LIGHTS = 4; // 最大ポイントライト数
			PTLight ptLights[MAX_POINT_LIGHTS];     // ポイントライト
			int numPointLights;                    // 今使っているポイントライトの数
			Vector3 pad7;                          // パディング(空けるだけ)


			LightCB()
				: directionLight(DirectionLight())
				, ambientLight(AmbientLight())
				, light(Light())
				, mLVP(Matrix::Identity)
				, shadowParam(ShadowParam())
				, numPointLights(0)
			{}
		};
		
		/**
		 * @brief シーンライトのインスタンスを取得
		 * @return シーンライトのインスタンス
		 * @details: SceneLight::GetInstance()を呼ぶことで「唯一の1個」(←staticなので) にアクセスが可能になる
		 */
		static SceneLight& GetInstance()
		{
			static SceneLight instance; // 最初に呼ばれた時に「1回」だけ生成される
			return instance;
		}

		//===============================================//
		// ライトのAPI
		//===============================================//
		/**
		 * @brief ライトの方向を設定(Vector3用)
		 * @param dir ライトの方向
		 */
		void SetDirection(const Vector3& dir)
		{
			m_lightCB.directionLight.direction = dir;
			// 一旦正規化しておく
			m_lightCB.directionLight.direction.Normalize();
		}

		/**
		 * @brief ライトの色を設定(Vector3用)
		 * @param color ライトの色
		 */
		void SetColor(const Vector3& color)
		{
			m_lightCB.directionLight.color = color;
		}

		/**
		 * @brief 環境光を設定(Vector3用)
		 * @param ambient 環境光の色
		 */
		void SetAmbient(const Vector3& ambient)
		{
			m_lightCB.ambientLight.ambient = ambient;
		}

		/**
		 * @brief ライトビュープロジェクション行列を設定
		 * @param mLVP ライトビュープロジェクション行列
		 */
		void SetLightProjMatrix(const Matrix& mlvp)
		{
			m_lightCB.mLVP = mlvp;
		}

		/**
		 * @brief シャドウバイアスを設定
		 * @param bias シャドウバイアス
		 */
		void SetShadowBias(float bias)
		{
			m_lightCB.shadowParam.shadowBias = bias;
		}

		/**
		 * @brief シャドウバイアスの最小値を設定
		 * @param biasMin シャドウバイアスの最小値
		 */
		void SetShadowBiasMin(float biasMin)
		{
			m_lightCB.shadowParam.shadowBiasMin = biasMin;
		}

		/**
		 * @brief ライトの定数バッファを取得
		 * @return ライトの定数バッファ
		 * @details: b1に渡す構造体
		 */
		LightCB& GetLightCB() { return m_lightCB; }


	private:
		/**
		 * @brief コンストラクタ
		 * @details; privateにすることで、外部からのインスタンス化を防ぐことができる
		 */
		SceneLight() {}


		/** ライトの定数バッファ */
		LightCB m_lightCB;
	};
}