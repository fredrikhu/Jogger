#include "MainWindow.h"
#include "misc.h"
#include "com.h"
#include "Rect.h"

constexpr wchar_t CLASS_NAME[] = L"MainWindow";
constexpr UINT_PTR ID_EXIT_AFTER_LAUNCH_TIMER = 1;

constexpr int ID_OK = 1001;
constexpr int ID_EDIT = 1002;
constexpr int ID_BROWSE = 1003;

bool MainWindow::Create(HINSTANCE hInstance) {
	hInstance_ = hInstance;

	const DWORD windowStyle = 0;
	const DWORD windowExstyle = WS_EX_LAYERED;
	const UINT dpi = GetDpiForSystem();

	hwnd_ = CreateWindowExW(
		windowExstyle,
		CLASS_NAME,
		L"Learn to Program Windows",
		windowStyle,

		CW_USEDEFAULT, CW_USEDEFAULT, 320, 72,

		nullptr,
		nullptr,
		hInstance,
		this
	);
	if (hwnd_ == nullptr) {
		return false;
	}

	RECT rect = scaler_.Scale(RECT { 0, 0, 320, 72 });
	AdjustWindowRectExForDpi(
		&rect,
		windowStyle,
		FALSE,
		0,
		dpi
	);
	SetWindowPos(
		hwnd_,
		nullptr,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREPOSITION
	);

	BOOL result = true;

	result &= SetWindowLongW(hwnd_, GWL_STYLE, windowStyle) != 0;
	result &= SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
	result &= SetWindowPos(
		hwnd_,
		NULL,
		0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
	);

	return result;
}

// TODO: We should be able to move a lot of this to the base class?
bool MainWindow::RegisterWindowClass(HINSTANCE hInstance) {
	const WNDCLASSW wc = {
		.lpfnWndProc = BaseWindow<MainWindow>::WindowProc,
		.hInstance = hInstance,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.lpszClassName = CLASS_NAME,
	};
	if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return false;
	}

	return SuggestionList::RegisterWindowClass(hInstance);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		if (!scaler_.Attach(hwnd_)) return -1;
		if (!d2d_.Attach(hwnd_)) return -1;
		return CreateControls();
	case WM_SIZE: {
		const UINT width = LOWORD(lParam);
		const UINT height = HIWORD(lParam);
		d2d_.ResizeRenderTarget(width, height);
		ResizeControls();
		suggestionList_.Reposition();
		break;
	}
	case WM_CTLCOLORBTN:
		CreateBackgroundBrush();
		return reinterpret_cast<LRESULT>(backgroundBrush_);
	case WM_CTLCOLOREDIT:
		SetTextColor((HDC)wParam, RGB(240, 240, 240));
		SetBkColor((HDC)wParam, RGB(
			backgroundColor_.r * 255,
			backgroundColor_.g * 255,
			backgroundColor_.b * 255
		));
		SetBkMode((HDC)wParam, OPAQUE);
		CreateBackgroundBrush();
		return (LRESULT)backgroundBrush_;
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
			const bool shouldShow = suggestionList_.UpdateSuggestions(query);
			const bool isShowing = IsWindowVisible(suggestionList_.Window());
			if (shouldShow && !isShowing) {
				suggestionList_.Show(SW_SHOWNOACTIVATE);
			}
			if (!shouldShow && isShowing) {
				suggestionList_.Hide();
			}
			if (shouldShow) {
				suggestionList_.Resize();
			}
			return 0;
		}
		break;
	case WM_MOVE: {
		const bool isShowing = IsWindowVisible(suggestionList_.Window());
		if (isShowing) {
			suggestionList_.Reposition();
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
		DeleteObject(backgroundBrush_);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd_, uMsg, wParam, lParam);
}

void MainWindow::CreateBackgroundBrush() {
	if (!backgroundBrush_) {
		backgroundBrush_ = CreateSolidBrush(RGB(
			backgroundColor_.r * 255,
			backgroundColor_.g * 255,
			backgroundColor_.b * 255
		));
	}
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
	if (msg.message == WM_KEYDOWN && msg.hwnd == edit_
		&& msg.wParam == VK_BACK && (GetKeyState(VK_CONTROL) & 0x8000)) {
		DWORD start, end;
		SendMessageW(edit_, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		if (start == end && start > 0) {
			auto text = GetText(edit_);
			int pos = static_cast<int>(start) - 1;
			while (pos > 0 && !iswalnum(text[pos - 1])) pos--;
			while (pos > 0 && iswalnum(text[pos - 1])) pos--;
			SendMessageW(edit_, EM_SETSEL, pos, start);
		}
		SendMessageW(edit_, WM_CLEAR, 0, 0);
		return true;
	}
	if (IsDialogMessageW(hwnd_, &msg)) {
		return true;
	}
	return false;
}

const Rect editRect{ 10, 10, 300, 20 };
const Rect okButtonRect{ 260, 39, 50, 28 };
const Rect browseButtonRect{ 155, 39, 100, 28 };
HRESULT MainWindow::CreateControls() {
	Rect rect = scaler_.Scale(editRect);
	edit_ = CreateWindowExW(
		0,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		rect.left, rect.top, rect.width, rect.height,
		hwnd_,
		ControlId(ID_EDIT),
		hInstance_,
		nullptr
	);
	if (!edit_) return -1;
	SetFocus(edit_);
	scaler_.SetScaledFont(edit_);
	rect = scaler_.Scale(okButtonRect);
	okButton_ = CreateWindowEx(
		0,
		L"BUTTON",
		L"Ok",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		rect.left, rect.top, rect.width, rect.height,
		hwnd_,
		ControlId(ID_OK),
		hInstance_,
		nullptr
	);
	if (!okButton_) return -1;
	scaler_.SetScaledFont(okButton_);
	rect = scaler_.Scale(browseButtonRect);
	browseButton_ = CreateWindowEx(
		0,
		L"BUTTON",
		L"Browse...",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		rect.left, rect.top, rect.width, rect.height,
		hwnd_,
		ControlId(ID_BROWSE),
		hInstance_,
		nullptr
	);
	if (!browseButton_) return -1;
	scaler_.SetScaledFont(browseButton_);
	if (!suggestionList_.Create(hInstance_, hwnd_)) return -1;
	suggestionList_.Attach(edit_);

	return 0;
}

void MainWindow::ResizeControls() {
	Rect rect = scaler_.Scale(editRect);
	SetWindowPos(edit_, nullptr, rect.left, rect.top, rect.width, rect.height, SWP_NONE);
	scaler_.SetScaledFont(edit_);
	rect = scaler_.Scale(okButtonRect);
	SetWindowPos(okButton_, nullptr, rect.left, rect.top, rect.width, rect.height, SWP_NONE);
	scaler_.SetScaledFont(okButton_);
	rect = scaler_.Scale(browseButtonRect);
	SetWindowPos(browseButton_, nullptr, rect.left, rect.top, rect.width, rect.height, SWP_NONE);
	scaler_.SetScaledFont(browseButton_);
	InvalidateRect(hwnd_, nullptr, FALSE);
}

void MainWindow::OnPaint() {
	auto pss = BeginPaint();
	ID2D1HwndRenderTarget* t;
	if (!(t = pss.RenderTarget())) return;

	t->Clear(backgroundColor_);
	auto size = t->GetSize();
	t->DrawRectangle(
		D2D1::RectF(0.5, 0.5, size.width - 0.5f, size.height - 0.5f),
		d2d_.AccentBrush().Get()
	);

	RECT rc{};
	RECT rcWin{};
	GetWindowRect(edit_, &rc);
	GetWindowRect(hwnd_, &rcWin);
	D2D1_RECT_F rcf = {
		.left = static_cast<FLOAT>(rc.left - rcWin.left),
		.top = static_cast<FLOAT>(rc.top - rcWin.top),
		.right = static_cast<FLOAT>(rc.right - rcWin.left),
		.bottom = static_cast<FLOAT>(rc.bottom - rcWin.top),
	};
	rcf = scaler_.Descale(rcf);
	t->DrawRectangle(
		rcf,
		d2d_.AccentBrush().Get()
	);
}
