#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include "com.h"
#include "d2d.h"
#include "MainWindow.h"

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