/**
 * @file LightController.cpp
 * @brief ステージの明かり(ライトの設定と、影を作る範囲)
 */
#include "stdafx.h"
#include "LightController.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/Camera/FinderController.h"
#include "Source/Util/RayCast.h"


namespace app
{
	namespace stage
	{
		namespace
		{
			/** 明かりのパラメーターのパス */
			constexpr const char* LIGHT_PARAMETER_PATH = "Assets/parameter/Light/Light.json";
			/** ファインダー中、影の中心を探すレイの長さ */
			constexpr float SHADOW_CENTER_RAY_LENGTH = 3000.0f;
		}


		LightController::LightController()
			: m_parameter()
			, m_photoCapture(nullptr)
			, m_finderController(nullptr)
			, m_boy(nullptr)
		{}


		LightController::~LightController()
		{}


		void LightController::Initialize(PhotoCapture* photoCapture, const camera::FinderController* finderController, const actor::Boy* boy)
		{
			m_parameter.Initialize(LIGHT_PARAMETER_PATH);
			m_photoCapture = photoCapture;
			m_finderController = finderController;
			m_boy = boy;
		}


		void LightController::Update()
		{
			// 明かりの色と向き(JSON を書き換えると、ホットリロードで値がすぐ反映する)
			const LightParameter& light = m_parameter.Get();
			SceneLight& sceneLight = SceneLight::GetInstance();
			sceneLight.SetColor(light.directionColor);
			sceneLight.SetAmbient(light.ambient);
			sceneLight.SetDirection(light.directionDir);

			// 影は今見ている所のまわりに作る
			// ※ ライトカメラは今のライトの向きで置くので、向きを渡した後に呼ぶ
			RenderingEngine::GetInstance().SetShadowCenter(CalcShadowCenter());
		}


		Vector3 LightController::CalcShadowCenter() const
		{
			// ふだんは主人公のまわり
			const Vector3& boyPosition = m_boy->GetTransform().GetWorldPosition();
			if (!m_photoCapture->IsFinderActive()) return boyPosition;

			// ピントを合わせている被写体がいれば、その被写体のまわり
			if (const camera::IPhotoSubject* subject = m_finderController->GetFocusSubject())
			{
				return subject->GetFocusPosition();
			}

			// いなければ、ファインダーの正面で当たった所のまわり(覗いている先の床・展示物)
			Camera& finderCamera = m_photoCapture->GetFinderCamera();
			const Vector3& eye = finderCamera.GetPosition();
			Vector3 end = finderCamera.GetForward();
			end.Scale(SHADOW_CENTER_RAY_LENGTH);
			end.Add(eye);
			// 主人公の体(カメラのすぐ近く)には当たらないようにする
			const util::RayHit hit = util::RayCastClosest(eye, end, m_boy->GetCollisionObject());
			if (hit.isHit) return hit.position;

			// 何にも当たらなければ主人公
			return boyPosition;
		}
	}
}
