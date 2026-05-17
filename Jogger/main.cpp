#include <string>
#include <Windows.h>
#include <shellapi.h>
#include "com.h"

constexpr wchar_t CLASS_NAME[] = L"MainWindow";
constexpr int ID_OK = 1001;
constexpr int ID_EDIT = 1002;
constexpr int ID_BROWSE = 1003;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
struct MainWindowState {
	HWND edit = nullptr;
	HWND okButton = nullptr;
	HWND browseButton = nullptr;
};

MainWindowState g_mainWindow{};

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	if (InitializeCom() == -1) return -1;

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	const WNDCLASSW wc = {
		.lpfnWndProc = WindowProc,
		.hInstance = hInstance,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
		.lpszClassName = CLASS_NAME,
	};

	if (!RegisterClassW(&wc)) {
		return -1;
	}

	const DWORD windowStyle = 0;
	const UINT dpi = GetDpiForSystem();
	RECT rect = { 0, 0, 320, 72 };
	AdjustWindowRectExForDpi(
		&rect,
		windowStyle,
		FALSE,
		0,
		dpi
	);

	const HWND hwnd = CreateWindowExW(
		0,
		CLASS_NAME,
		L"Learn to Program Windows",
		windowStyle,

		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,

		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	if (hwnd == nullptr) {
		return -1;
	}
	SetWindowLongW(hwnd, GWL_STYLE, windowStyle);

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN && (msg.hwnd == g_mainWindow.edit)) {
			SendMessageW(
				hwnd,
				WM_COMMAND,
				MAKEWPARAM(ID_OK, BN_CLICKED),
				reinterpret_cast<LPARAM>(g_mainWindow.okButton)
			);
			continue;
		}
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
			continue;
		}
		if (IsDialogMessageW(hwnd, &msg)) {
			continue;
		}

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	UninitializeCom();
	return static_cast<int>(msg.wParam);
}

LRESULT CreateControls(HWND hwnd);
bool LaunchWithShell(
	HWND owner,
	const std::wstring& command
) {
	SHELLEXECUTEINFOW info{
		.cbSize = sizeof(info),
		.hwnd = owner,
		.lpVerb = L"open",
		.lpFile = command.c_str(),
		.nShow = SW_SHOWNORMAL,
	};
	return ShellExecuteExW(&info) != FALSE;
}

std::wstring GetText(HWND hwnd) {
	const int length = GetWindowTextLengthW(hwnd);

	std::wstring text(length + 1, L'\0');
	const int copied = GetWindowTextW(hwnd, text.data(), static_cast<int>(text.size()));

	text.resize(copied);
	return text;
}

constexpr UINT_PTR ID_EXIT_AFTER_LAUNCH_TIMER = 1;
LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
) {
	switch (uMsg) {
	case WM_CREATE:
		return CreateControls(hwnd);
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OK && HIWORD(wParam) == BN_CLICKED) {
			const auto command = GetText(g_mainWindow.edit);

			if (LaunchWithShell(hwnd, command)) {
				ShowWindow(hwnd, SW_HIDE);
				SetTimer(hwnd, ID_EXIT_AFTER_LAUNCH_TIMER, 250, nullptr);
			}
			return 0;
		}
		if (LOWORD(wParam) == ID_BROWSE && HIWORD(wParam) == BN_CLICKED) {
			std::wstring filePath;
			if (!PickFile(hwnd, filePath)) return 0;

			SetWindowTextW(hwnd, filePath.c_str());
			SetFocus(g_mainWindow.edit);
			SendMessageW(g_mainWindow.edit, EM_SETSEL, filePath.size(), filePath.size());

			return 0;
		}
		break;
	case WM_TIMER:
		if (wParam == ID_EXIT_AFTER_LAUNCH_TIMER) {
			KillTimer(hwnd, ID_EXIT_AFTER_LAUNCH_TIMER);
			DestroyWindow(hwnd);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HMENU ControlId(int id) {
	return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
}

LRESULT CreateControls(HWND hwnd) {
	const HINSTANCE hInstance = GetModuleHandleW(nullptr);
	g_mainWindow.edit = CreateWindowExW(
		0,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
		10, 10, 300, 24,
		hwnd,
		ControlId(ID_EDIT),
		hInstance,
		nullptr
	);
	if (!g_mainWindow.edit) return -1;
	SetFocus(g_mainWindow.edit);
	g_mainWindow.okButton = CreateWindowEx(
		0,
		L"BUTTON",
		L"Ok",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		260, 39, 50, 28,
		hwnd,
		ControlId(ID_OK),
		hInstance,
		nullptr
	);
	if (!g_mainWindow.okButton) return -1;
	g_mainWindow.browseButton = CreateWindowEx(
		0,
		L"BUTTON",
		L"Browse...",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		155, 39, 100, 28,
		hwnd,
		ControlId(ID_BROWSE),
		hInstance,
		nullptr
	);
	if (!g_mainWindow.browseButton) return -1;

	return 0;
}