#include <string>
#include <Windows.h>
#include <shellapi.h>
#include <vector>
#include "com.h"
#include "d2d.h"
#include "MainWindow.h"

constexpr wchar_t SUGGESTION_LIST_CLASS[] = L"SuggestionList";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SuggestionListProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
struct MainWindowState {
	HWND edit = nullptr;
	HWND okButton = nullptr;
	HWND browseButton = nullptr;
	HWND suggestionList = nullptr;

	int hoveredIndex = -1;
	int selectedIndex = 0;
	std::vector<std::wstring> visibleSuggestions;
};

std::vector<std::wstring> allSuggestions{
	L"notepad",
	L"calc",
	L"explorer"
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
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN && (msg.hwnd == g_mainWindow.edit)) {
			SendMessageW(
				mainWindow.Window(),
				WM_COMMAND,
				MAKEWPARAM(ID_OK, BN_CLICKED),
				reinterpret_cast<LPARAM>(g_mainWindow.okButton)
			);
			continue;
		}
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
			DestroyWindow(mainWindow.Window());
			continue;
		}
		if (IsDialogMessageW(mainWindow.Window(), &msg)) {
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

void OnPaint(HWND hwnd) {
	PAINTSTRUCT ps;
	HRESULT hr;
	D2D1_SIZE_F size;
	ID2D1HwndRenderTarget* t = nullptr;
	BeginPaint(hwnd, &ps);
	if (FAILED(EnsureRenderTarget(hwnd))) goto END_PAINT;

	t = renderTarget.Get();
	t->BeginDraw();
	t->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
	size = t->GetSize();
	t->DrawRectangle(
		D2D1::RectF(0.5, 0.5, size.width-0.5f, size.height-0.5f),
		backgroundBrush.Get()
	);
	hr = renderTarget.Get()->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET) {
		renderTarget.Reset();
		backgroundBrush.Reset();
	}

	END_PAINT:
	EndPaint(hwnd, &ps);
}

void UpdateSuggestions(const std::wstring& text) {
	g_mainWindow.visibleSuggestions.clear();
	if (text.length() == 0) return;
	for (const auto s : allSuggestions) {
		if (s.starts_with(text)) {
			g_mainWindow.visibleSuggestions.push_back(s);
		}
	}
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

