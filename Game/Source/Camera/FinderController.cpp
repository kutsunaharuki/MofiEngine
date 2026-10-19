/**
 * @file FinderController.cpp
 * @brief ファインダー(写真撮影)の操作
 */
#include "stdafx.h"
#include "FinderController.h"


namespace app
{
	namespace camera
	{
		namespace
		{
			/** 調整用パラメーターの JSON */
			constexpr const char* FINDER_PARAMETER_PATH = "Assets/parameter/Camera/Finder.json";
		}


		FinderController::FinderController()
			: m_photoCapture(nullptr)
			, m_coolTimer(0.0f)
		{}


		FinderController::~FinderController()
		{}


		void FinderController::Initialize(PhotoCapture* photoCapture)
		{
			m_photoCapture = photoCapture;
			m_parameter.Initialize(FINDER_PARAMETER_PATH);
			m_coolTimer = 0.0f;
		}


		bool FinderController::Update()
		{
			if (m_photoCapture == nullptr) return false;

			// クールタイムは、覗いていなくても進める
			m_coolTimer -= g_gameTime->GetFrameDeltaTime();
			if (m_coolTimer < 0.0f) m_coolTimer = 0.0f;

			// 覗く / やめる
			if (g_pad[0]->IsTrigger(enButtonY))
			{
				m_photoCapture->ToggleFinderActive();
			}
			if (!m_photoCapture->IsFinderActive()) return false;

			const FinderParameter& parameter = m_parameter.Get();

			// ピント(上で遠く、下で近く)
			if (g_pad[0]->IsPress(enButtonUp))
			{
				m_photoCapture->FinderFocus(parameter.focusSpeed);
			}
			if (g_pad[0]->IsPress(enButtonDown))
			{
				m_photoCapture->FinderFocus(-parameter.focusSpeed);
			}

			// ズーム(RB1 で寄る、LB1 で引く)
			if (g_pad[0]->IsPress(enButtonRB1))
			{
				m_photoCapture->FinderZoom(parameter.zoomSpeed);
			}
			if (g_pad[0]->IsPress(enButtonLB1))
			{
				m_photoCapture->FinderZoom(-parameter.zoomSpeed);
			}

			// 撮影(クールタイム中は撮れない)
			if (g_pad[0]->IsTrigger(enButtonB) && CanShot())
			{
				m_photoCapture->Shot();
				m_coolTimer = parameter.shotCoolTime;
				return true;
			}
			return false;
		}
	}
}
