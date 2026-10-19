/**
 * @file FinderController.h
 * @brief ファインダー(写真撮影)の操作
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ファインダーの調整用パラメーター
		 * @details JSON : Assets/parameter/Camera/Finder.json(保存すると、ゲームを動かしたまま反映される)
		 */
		struct FinderParameter
		{
			/** ズームの速さ(1.0 がエンジンの標準の速さ) */
			float zoomSpeed = 1.0f;
			/** ピントを動かす速さ(1.0 がエンジンの標準の速さ) */
			float focusSpeed = 1.0f;
			/** 撮ってから、次に撮れるようになるまでの時間(秒) */
			float shotCoolTime = 1.0f;

			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				zoomSpeed = file.GetFloat("zoomSpeed", zoomSpeed);
				focusSpeed = file.GetFloat("focusSpeed", focusSpeed);
				shotCoolTime = file.GetFloat("shotCoolTime", shotCoolTime);
			}
		};


		/**
		 * @brief ファインダー(写真撮影)の操作
		 * @details 操作 : Y で覗く / やめる、十字の上下でピント、RB1 / LB1 でズーム、B で撮影。
		 *          撮影には、クールタイムがある
		 */
		class FinderController : public Noncopyable
		{
		public:
			FinderController();
			~FinderController();

			/**
			 * @brief 初期化
			 * @param photoCapture 操作する写真撮影クラス
			 */
			void Initialize(PhotoCapture* photoCapture);

			/**
			 * @brief 更新処理(入力を受け付ける)
			 * @return このフレームで写真を撮ったら true
			 */
			bool Update();


		public:
			/**
			 * @brief 今、撮影できるか(クールタイムが終わっているか)
			 * @return 撮影できれば true
			 */
			bool CanShot() const { return m_coolTimer <= 0.0f; }
			/**
			 * @brief クールタイムの残りを取得
			 * @return 残りの秒数(0 なら撮影できる)
			 */
			float GetCoolTimeRemaining() const { return m_coolTimer; }


		private:
			/** 操作する写真撮影クラス */
			PhotoCapture* m_photoCapture;
			/** 調整用パラメーター */
			util::Parameter<FinderParameter> m_parameter;
			/** クールタイムの残り(秒) */
			float m_coolTimer;
		};
	}
}
