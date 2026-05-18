#include "MainWindow.h"
#include "misc.h"
#include "com.h"

constexpr wchar_t CLASS_NAME[] = L"MainWindow";
constexpr UINT_PTR ID_EXIT_AFTER_LAUNCH_TIMER = 1;

constexpr int ID_OK = 1001;
constexpr int ID_EDIT = 1002;
constexpr int ID_BROWSE = 1003;

std::vector<std::wstring> allSuggestions{
	L"notepad",
	L"calc",
	L"explorer"
};

bool MainWindow::Create(HINSTANCE hInstance) {
	hInstance_ = hInstance;

	const DWORD windowStyle = 0;
	const DWORD windowExstyle = WS_EX_LAYERED;
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
		windowExstyle,
		CLASS_NAME,
		L"Learn to Program Windows",
		windowStyle,

		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,

		nullptr,
		nullptr,
		hInstance,
		this
	);
	if (hwnd == nullptr) {
		return false;
	}

	BOOL result = true;

	result &= SetWindowLongW(hwnd, GWL_STYLE, windowStyle) != 0;
	result &= SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA);
	result &= SetWindowPos(
		hwnd,
		NULL,
		0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
	);

	result &= static_cast<BOOL>(suggestionList_.Create(hInstance, hwnd_));

	return result;
}

bool MainWindow::Register(HINSTANCE hInstance) {
	const WNDCLASSW wc = {
		.lpfnWndProc = BaseWindow<MainWindow>::WindowProc,
		.hInstance = hInstance,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.lpszClassName = CLASS_NAME,
	};
	BOOL success = true;
	if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return false;
	}

	return SuggestionList::Register(hInstance);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		if (!d2d_.CreateD2DFactory()) return -1;
		return CreateControls();
	case WM_SIZE:
		if (d2d_.RenderTarget().Get()) {
			const UINT width = LOWORD(lParam);
			const UINT height = HIWORD(lParam);
			d2d_.ResizeRenderTarget(width, height);
		}
		break;
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OK && HIWORD(wParam) == BN_CLICKED) {
			const auto command = GetText(edit_);

			if (LaunchWithShell(hwnd_, command)) {
				ShowWindow(hwnd_, SW_HIDE);
				SetTimer(hwnd_, ID_EXIT_AFTER_LAUNCH_TIMER, 250, nullptr);
			}
			return 0;
		}
		if (LOWORD(wParam) == ID_BROWSE && HIWORD(wParam) == BN_CLICKED) {
			std::wstring filePath;
			if (!PickFile(hwnd_, filePath)) return 0;

			SetWindowTextW(edit_, filePath.c_str());
			SetFocus(edit_);
			SendMessageW(edit_, EM_SETSEL, filePath.size(), filePath.size());

			return 0;
		}
		if (LOWORD(wParam) == ID_EDIT && HIWORD(wParam) == EN_CHANGE) {
			const auto query = GetText(edit_);
			UpdateSuggestions(query);
			const bool shouldShow = !visibleSuggestions_.empty();
			const bool isShowing = IsWindowVisible(suggestionList_.Window());
			if (shouldShow && !isShowing) {
				RECT rc{};
				GetWindowRect(edit_, &rc);
				SetWindowPos(
					suggestionList_.Window(), nullptr,
					rc.left, rc.bottom,
					0, 0,
					SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW
				);
			}
			if (!shouldShow && isShowing) {
				ShowWindow(suggestionList_.Window(), SW_HIDE);
			}
			return 0;
		}
		break;
	case WM_MOVE: {
		const bool isShowing = IsWindowVisible(suggestionList_.Window());
		if (isShowing) {
			RECT rc{};
			GetWindowRect(edit_, &rc);
			SetWindowPos(
				suggestionList_.Window(), nullptr,
				rc.left, rc.bottom,
				0, 0,
				SWP_NOSIZE | SWP_NOACTIVATE
			);
		}
		return 0;
	}
	case WM_TIMER:
		if (wParam == ID_EXIT_AFTER_LAUNCH_TIMER) {
			KillTimer(hwnd_, ID_EXIT_AFTER_LAUNCH_TIMER);
			Destroy();
			return 0;
		}
		break;
	case WM_DESTROY:
		suggestionList_.Destroy();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd_, uMsg, wParam, lParam);
}

bool MainWindow::HandleMessage(MSG& msg) {
	if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN && (msg.hwnd == edit_)) {
		SendMessageW(
			hwnd_,
			WM_COMMAND,
			MAKEWPARAM(ID_OK, BN_CLICKED),
			reinterpret_cast<LPARAM>(okButton_)
		);
		return true;
	}
	if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
		DestroyWindow(hwnd_);
		return true;
	}
	if (IsDialogMessageW(hwnd_, &msg)) {
		return true;
	}
	return false;
}

HRESULT MainWindow::CreateControls() {
	edit_ = CreateWindowExW(
		0,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
		10, 10, 300, 24,
		hwnd_,
		ControlId(ID_EDIT),
		hInstance_,
		nullptr
	);
	if (!edit_) return -1;
	SetFocus(edit_);
	okButton_ = CreateWindowEx(
		0,
		L"BUTTON",
		L"Ok",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		260, 39, 50, 28,
		hwnd_,
		ControlId(ID_OK),
		hInstance_,
		nullptr
	);
	if (!okButton_) return -1;
	browseButton_ = CreateWindowEx(
		0,
		L"BUTTON",
		L"Browse...",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		155, 39, 100, 28,
		hwnd_,
		ControlId(ID_BROWSE),
		hInstance_,
		nullptr
	);
	if (!browseButton_) return -1;

	return 0;
}


void MainWindow::OnPaint() {
	PAINTSTRUCT ps;
	HRESULT hr;
	D2D1_SIZE_F size;
	ID2D1HwndRenderTarget* t = nullptr;
	BeginPaint(hwnd_, &ps);
	if (FAILED(d2d_.EnsureRenderTarget(hwnd_))) goto END_PAINT;

	t = d2d_.RenderTarget().Get();
	t->BeginDraw();
	t->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
	size = t->GetSize();
	t->DrawRectangle(
		D2D1::RectF(0.5, 0.5, size.width - 0.5f, size.height - 0.5f),
		d2d_.BackgroundBrush().Get()
	);
	hr = t->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET) {
		d2d_.RenderTarget().Reset();
		d2d_.BackgroundBrush().Reset();
	}

END_PAINT:
	EndPaint(hwnd_, &ps);
}


void MainWindow::UpdateSuggestions(const std::wstring& text) {
	visibleSuggestions_.clear();
	if (text.length() == 0) return;
	for (const auto s : allSuggestions) {
		if (s.starts_with(text)) {
			visibleSuggestions_.push_back(s);
		}
	}
}
