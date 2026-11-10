/**
 * @file LightController.h
 * @brief ステージの明かり(ライトの設定と、影を作る範囲)
 */
#pragma once
#include "Source/Stage/Object/LightParameter.h"
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		class Boy;
	}
	namespace camera
	{
		class FinderController;
	}


	namespace stage
	{
		/**
		 * @brief ステージの明かり
		 * @details Light.json の色・環境光・向きを SceneLight に渡す。
		 *          影は「今見ている所」のまわりに作る(ふだんは主人公、ファインダー中は撮っている相手)
		 */
		class LightController : public Noncopyable
		{
		public:
			LightController();
			~LightController();

			/**
			 * @brief 初期化
			 * @param photoCapture ファインダー(覗いているか・カメラの位置)
			 * @param finderController ピントを合わせている被写体を聞く相手
			 * @param boy 主人公
			 */
			void Initialize(PhotoCapture* photoCapture, const camera::FinderController* finderController, const actor::Boy* boy);

			/**
			 * @brief 更新
			 * @details 被写体が決まった後(FinderController::LateUpdate の後)に呼ぶ
			 */
			void Update();


		private:
			/**
			 * @brief 影を作る範囲の中心を決める
			 * @return ファインダー中はピントを合わせている所(被写体 → 正面で当たった所)、それ以外は主人公
			 */
			Vector3 CalcShadowCenter() const;


		private:
			/** 明かりの設定(Light.json) */
			util::Parameter<LightParameter> m_parameter;
			/** ファインダー */
			PhotoCapture* m_photoCapture;
			/** ピントを合わせている被写体を聞く相手 */
			const camera::FinderController* m_finderController;
			/** 主人公 */
			const actor::Boy* m_boy;
		};
	}
}
