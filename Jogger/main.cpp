#include <string>
#include <Windows.h>
#include <shellapi.h>
#include <vector>
#include "com.h"
#include "d2d.h"
#include "MainWindow.h"

constexpr wchar_t SUGGESTION_LIST_CLASS[] = L"SuggestionList";

LRESULT CALLBACK SuggestionListProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	if (FAILED(InitializeCom())) return -1;

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	MainWindow::Register(hInstance);
	MainWindow mainWindow{};
	mainWindow.Create(hInstance);

	const WNDCLASSW suggestionClass = {
		.lpfnWndProc = SuggestionListProc,
		.hInstance = hInstance,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
		.lpszClassName = SUGGESTION_LIST_CLASS,
	};
	if (!RegisterClassW(&suggestionClass)) {
		return -1;
	}

	mainWindow.Show(SW_SHOW);

	MSG msg = {};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
		if (mainWindow.HandleMessage(msg)) continue;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	UninitializeCom();
	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK SuggestionListProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
) {
	/*switch (uMsg) {
	case WM_PAINT:
		return 0;
	}*/
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

