#include "stdafx.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"
#include "system/system.h"

#include<InitGUID.h>
#include<dxgidebug.h>

#include "Game.h"
#include "Source/Util/ProfilerWindow.h"



void ReportLiveObjects()
{
	IDXGIDebug* pDxgiDebug;

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);

	pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
}

///////////////////////////////////////////////////////////////////
// Windows application entry point.
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Initialize the game (window + K2EngineLow).
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
	//////////////////////////////////////
	// Write your own start-up code from here.
	//////////////////////////////////////

	// Create the Game object.
	NewGO<Game>(0, "game");

	RenderingEngine::GetInstance().InitializeLight();
	//////////////////////////////////////
	// End of start-up code.
	//////////////////////////////////////

	// 処理時間(CPU・GPU)を表示するウィンドウ(F1 で表示・非表示)
	app::util::ProfilerWindow profilerWindow;
	Profiler& profiler = Profiler::GetInstance();

	// Game loop.
	while (DispatchWindowMessage())
	{
		if (g_pad[0]->IsTrigger(enButtonA)) {
			g_pad[0]->SetVibration(/*durationSec=*/0.5f, /*normalizedPower=*/1.0f);
		}

		// Drive one frame of the low-level engine yourself.
		// (K2Engine used to hide this behind K2Engine::Execute().)
		g_engine->BeginFrame();		// Begin the frame: clear the screen, update input, etc.
		// 計測のフレーム開始(コマンドリストの記録が始まった後に呼ぶ)
		profiler.BeginFrame();

		// imguiのフレーム開始
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// ギズモ(ステージエディタで使う)のフレーム開始。ImGui::NewFrame の後に呼ぶ
		ImGuizmo::BeginFrame();

		{
			K2_PROFILE_CPU("Update");
			g_engine->ExecuteUpdate();	// Update all game objects (IGameObject::Update).
		}
		{
			// ゲームオブジェクトの Render は、描くモデルを登録するだけ(実際に描くのは RenderingEngine)
			K2_PROFILE_CPU("Render (GameObject)");
			g_engine->ExecuteRender();	// Render all game objects (IGameObject::Render).
		}
		{
			K2_PROFILE_CPU("RenderingEngine");
			RenderingEngine::GetInstance().Execute(g_graphicsEngine->GetRenderContext());
		}

		// デモウィンドウを表示
		//ImGui::ShowDemoWindow();

		// 処理時間のウィンドウ(ImGui::Render の前に呼ぶ)
		profilerWindow.Draw();

		{
			K2_PROFILE("ImGui");
			// imguiの描画(フレームの一番最後 = 全部の絵の上に重ねる)
			ImGui::Render();
			auto* cmdList = g_graphicsEngine->GetCommandList();
			// imguiのSRVヒープを設定する(設定しないと表示されない)
			cmdList->SetDescriptorHeaps(1, &g_imguiSrvHeap);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
		}

		// GPU の計測を閉じる(コマンドリストを閉じる EndFrame の前に呼ぶ)
		profiler.EndGpuFrame();

		{
			// 画面に出して、前のフレームの GPU の処理が終わるのを待つ
			// ※ ここが長い時は、CPU が GPU を待っている = GPU が重い
			K2_PROFILE_CPU("EndFrame (Present + Wait GPU)");
			g_engine->EndFrame();		// End the frame: present the back buffer.
		}
	}

	// Shut down.
	FinalizeGame();

	// 計測用の GPU のリソースを解放する
	// ※ GPU がまだ使っているかもしれないので、GPU の処理が終わる FinalizeGame の後にする
	profiler.Release();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}
