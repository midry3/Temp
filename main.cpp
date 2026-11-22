#include <Novot.h>
#include "Game/Board/MapEditor.h"
#include "Game/Battle/TestBattle.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <Windows.h>
#include <DxLib.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	LRESULT CALLBACK MapEditorWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (ImGui::GetCurrentContext()
			&& ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return TRUE;
		return FALSE;
	}
}

void NovotSetup() {
	SetHookWinProc(MapEditorWndProc);
	SceneManager.SetCurrentScene<Game::MapEditor>();
}

int NovotMain() {
	PropertyManager.SetWindowSize({1280, 720});
	PropertyManager.SetFPS(60);

	return Novot::StartGame();
}
