#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include "com.h"
#include "d2d.h"
#include "MainWindow.h"
#include "DpiScaler.h"

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	DpiScaler::Initialize();

	ComInitializer initializer{};
	if (!initializer.IsInitialized()) return -1;
	if (!D2D::CreateD2DFactory()) return -1;

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	if (!MainWindow::Register(hInstance)) return -1;
	MainWindow mainWindow{};
	if (!mainWindow.Create(hInstance)) return -1;

	mainWindow.Show(SW_SHOW);

	MSG msg = {};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
		if (mainWindow.HandleMessage(msg)) continue;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return static_cast<int>(msg.wParam);
}