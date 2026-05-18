#include "SuggestionList.h"

constexpr wchar_t SUGGESTION_LIST_CLASS[] = L"SuggestionList";

bool SuggestionList::Register(HINSTANCE hInstance) {
	const WNDCLASSW suggestionClass = {
	.lpfnWndProc = BaseWindow<SuggestionList>::WindowProc,
	.hInstance = hInstance,
	.hCursor = LoadCursorW(nullptr, IDC_ARROW),
	.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
	.lpszClassName = SUGGESTION_LIST_CLASS,
	};
	if (RegisterClassW(&suggestionClass)) {
		return true;
	}

	return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT SuggestionList::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	/*switch (uMsg) {
	case WM_PAINT:
		return 0;
	}*/
	return DefWindowProcW(hwnd_, uMsg, wParam, lParam);
}

bool SuggestionList::Create(HINSTANCE hInstance, HWND owner) {
	hInstance_ = hInstance;

	hwnd_ = CreateWindowExW(
		WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
		L"SuggestionList",
		nullptr,
		WS_POPUP,
		10, 34, 300, 120,
		owner,
		nullptr,
		hInstance_,
		this
	);
	if (!hwnd_) return false;
	BOOL result = true;
	result &= SetLayeredWindowAttributes(hwnd_, 0, 200, LWA_ALPHA);
	result &= SetWindowPos(
		hwnd_,
		NULL,
		0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
	);

	return result;
 }