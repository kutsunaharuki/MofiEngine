/**
 * @file FlashDebugWindow.cpp
 * @brief 2D と 3D のフラッシュの Debug 調整ウィンドウ
 */
#include "stdafx.h"
#include "FlashDebugWindow.h"
#include "imgui.h"
#include "Source/Util/ParameterManager.h"
#include "Source/UI/TitleEventMenu/TitleEventMenu.h"


namespace app
{
	namespace debug
	{
		void FlashDebugWindow::Draw(ui::TitleEventMenu* titleEventMenu)
		{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
			if (!titleEventMenu) return;

			ui::TitleEventMenu::CameraFlashSetting& setting = titleEventMenu->GetCameraFlashSetting();
			
			ImGui::Begin("Title Flash");
			ImGui::SliderFloat("First Delay", &setting.firstDelay, 0.0f, 3.0f);
			ImGui::SliderFloat("Rise Time", &setting.riseTime, 0.01f, 0.5f);
			ImGui::SliderFloat("Fall Time", &setting.fallTime, 0.05f, 2.0f);
			ImGui::SliderFloat("Light Alpha", &setting.lightAlpha, 0.0f, 1.0f);
			ImGui::SliderFloat("Light Min Scale", &setting.lightMinScale, 0.0f, 3.0f);
			ImGui::SliderFloat("Light Max Scale", &setting.lightMaxScale, 0.0f, 3.0f);
			ImGui::SliderFloat("Screen Alpha", &setting.screenAlpha, 0.0f, 0.5f);
			
			// フラッシュをリプレイする
			if (ImGui::Button("Replay")) titleEventMenu->ReplayCameraFlash();
			ImGui::End();
#endif // APP_ENABLE_PARAMETER_HOTRELOAD
		}
	}
}