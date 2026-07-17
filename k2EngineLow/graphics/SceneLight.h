/**
 * @file SceneLight.h
 * @brief シーンライトクラス
 */
#pragma once


namespace nsK2EngineLow {


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


			Light()
				: eyePos(g_camera3D->GetPosition())
				, specPower(1.0f)
				, reflectPower(0.1f)
			{}
		};


		/**
		 * @brief ライトの定数バッファの構造体
		 * @details: これまで入れた物をまとめて定数バッファに入れるための構造体
		 */
		struct LightCB
		{
			DirectionLight directionLight;	// ディレクションライト
			AmbientLight ambientLight;		// 環境光
			Light light;					// ライト


			LightCB()
				: directionLight(DirectionLight())
				, ambientLight(AmbientLight())
				, light(Light())
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

		/**
		 * @brief ライトの方向を設定
		 * @param dir ライトの方向
		 */
		void SetDirection(const Vector3& dir);
		/**
		 * @brief ライトの色を設定
		 * @param color ライトの色
		 */
		void SetColor(const Vector3& color);
		/**
		 * @brief 環境光を設定
		 * @param ambient 環境光の色
		 */
		void SetAmbient(const Vector3& ambient);

		/**
		 * @brief ライトの定数バッファを取得
		 * @return ライトの定数バッファ
		 * @details: b1に渡す構造体
		 */
		LightCB& GetLightCB() { return lightCB_; }


	private:
		/**
		 * @brief コンストラクタ
		 * @details; privateにすることで、外部からのインスタンス化を防ぐことができる
		 */
		SceneLight() {}


		/** ライトの定数バッファ */
		LightCB lightCB_;
	};
}