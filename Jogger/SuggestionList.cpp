#include "SuggestionList.h"

constexpr wchar_t SUGGESTION_LIST_CLASS[] = L"SuggestionList";
const FLOAT SuggestionList::verticalPadding = 4.0f;

std::vector<std::wstring> allSuggestions{
	L"notepad",
	L"calc",
	L"explorer",
	L"totalcommander",
	L"edge"
};

bool SuggestionList::RegisterWindowClass(HINSTANCE hInstance) {
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
	switch (uMsg) {
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_SIZE: {
		const UINT width = LOWORD(lParam);
		const UINT height = HIWORD(lParam);
		d2d_.ResizeRenderTarget(width, height);
		InvalidateRect(hwnd_, nullptr, FALSE);
		return 0;
	}
	case WM_SHOWWINDOW:
		Reposition();
		Resize();
		break;
	}
	return DefWindowProcW(hwnd_, uMsg, wParam, lParam);
}

void SuggestionList::OnPaint() {
	auto pss = BeginPaint();
	ID2D1HwndRenderTarget* t;
	if (!(t = pss.RenderTarget())) return;

	t->Clear(D2D1::ColorF(0.118f, 0.118f, 0.118f));
	auto size = t->GetSize();
	t->DrawRectangle(
		D2D1::RectF(0.5, 0.5, size.width - 0.5f, size.height - 0.5f),
		d2d_.AccentBrush().Get()
	);

	int offset = 0;
	const FLOAT lineHeight = LineHeightDips();

	for (auto& s : visibleSuggestions_) {
		const FLOAT pos = lineHeight * (offset++) + verticalPadding / 2.0f;
		D2D1_RECT_F rect{
			.left = 8,
			.top = pos,
			.right = 300-16,
			.bottom = pos + lineHeight,
		};
		t->DrawTextW(
			s.c_str(),
			static_cast<UINT32>(s.size()),
			d2d_.TextFormat(),
			rect,
			d2d_.TextBrush().Get()
		);
	}
}

FLOAT SuggestionList::LineHeightDips() {
	auto metrics = d2d_.FontMetrics();
	const FLOAT fontSizeDips = 9.0f * 96.0f / 72.0f;
	return (metrics.ascent + metrics.descent)
		* fontSizeDips / metrics.designUnitsPerEm + verticalPadding;
}

void SuggestionList::Reposition() {
	RECT rc{};
	GetWindowRect(attachedTo_, &rc);
	SetWindowPos(
		hwnd_, nullptr,
		rc.left, rc.bottom,
		0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE
	);
}

void SuggestionList::Resize() {
	RECT rc{};
	GetWindowRect(attachedTo_, &rc);
	SetWindowPos(
		hwnd_,
		nullptr,
		rc.left, rc.bottom,
		rc.right - rc.left, CalculateHeight(),
		SWP_NOREPOSITION | SWP_NOACTIVATE
	);

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
	if (!scaler_.Attach(hwnd_)) return false;
	if (!d2d_.Attach(hwnd_)) return false;

	BOOL result = true;

	result &= SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
	result &= SetWindowPos(
		hwnd_,
		NULL,
		0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
	);

	return result;
 }

void SuggestionList::Attach(HWND hwnd) {
	attachedTo_ = hwnd;
}

bool SuggestionList::UpdateSuggestions(const std::wstring& text) {
	visibleSuggestions_.clear();
	if (text.length() == 0) return false;
	for (const auto s : allSuggestions) {
		if (s.contains(text)) {
			visibleSuggestions_.push_back(s);
		}
	}

	return !visibleSuggestions_.empty();
}

int SuggestionList::CalculateHeight() {
	const FLOAT lineHeight = LineHeightDips();
	const FLOAT totalHeightDips = visibleSuggestions_.size() * lineHeight;
	const FLOAT totalHeightPixels = scaler_.Scale(totalHeightDips);

	return static_cast<int>(totalHeightPixels + 0.5);
}