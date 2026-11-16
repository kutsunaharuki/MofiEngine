#pragma once

extern HWND			g_hWnd ;				// Window handle.

// externをつけるとグローバル変数になる
extern ID3D12DescriptorHeap* g_imguiSrvHeap;

// Initialize the game (creates the window and the low-level engine).
void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName);
// Shut down the game (destroys the low-level engine).
void FinalizeGame();
// Pump window messages. Returns false when the game should quit.
bool DispatchWindowMessage();
/**
 * @brief テクスチャを ImGui に登録して、ImGui::Image で出せるようにする
 * @param resource テクスチャ(描き先など)
 * @param format テクスチャのフォーマット
 * @return ImGui に渡す番号(ImTextureID)
 * @details imgui の SRV ヒープに窓口を 1 つ作る(ヒープは 64 個なので、同じテクスチャは 1 回だけ登録する)
 */
UINT64 RegisterImGuiTexture(ID3D12Resource* resource, DXGI_FORMAT format);
