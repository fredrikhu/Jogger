#include "SuggestionList.h"
#include "DpiScaler.h"

constexpr wchar_t SUGGESTION_LIST_CLASS[] = L"SuggestionList";
const FLOAT SuggestionList::verticalPadding = 4.0f;

std::vector<std::wstring> allSuggestions{
	L"notepad",
	L"calc",
	L"explorer"
};

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
	switch (uMsg) {
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_SIZE: {
		const UINT width = LOWORD(lParam);
		const UINT height = HIWORD(lParam);
		d2d_.ResizeRenderTarget(width, height);
		return 0;
	}
	}
	return DefWindowProcW(hwnd_, uMsg, wParam, lParam);
}

void SuggestionList::OnPaint() {
	auto pss = BeginPaint();
	ID2D1HwndRenderTarget* t;
	if (!(t = pss.RenderTarget())) return;

	t->Clear(D2D1::ColorF(D2D1::ColorF(0.118f, 0.118f, 0.118f)));
	auto size = t->GetSize();
	t->DrawRectangle(
		D2D1::RectF(0.5, 0.5, size.width - 0.5f, size.height - 0.5f),
		d2d_.AccentBrush().Get()
	);
	int offset = 0;
	for (auto& s : visibleSuggestions_) {
		auto metrics = d2d_.FontMetrics();
		const FLOAT lineHeight = (metrics.ascent + metrics.descent + metrics.lineGap)
			* DpiScaler::ScaleFontSize(9.0) / metrics.designUnitsPerEm + DpiScaler::Scale(verticalPadding);
		const FLOAT pos = lineHeight * (offset++);
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

	result &= SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
	result &= SetWindowPos(
		hwnd_,
		NULL,
		0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
	);

	return result;
 }

void SuggestionList::PositionBelow(HWND hwnd) {
	RECT rc{};
	GetWindowRect(hwnd, &rc);
	SetWindowPos(
		hwnd_, nullptr,
		rc.left, rc.bottom,
		0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE
	);
}

void SuggestionList::ShowBelow(HWND hwnd) {
	RECT rc{};
	GetWindowRect(hwnd, &rc);
	SetWindowPos(
		hwnd_, nullptr,
		rc.left, rc.bottom,
		0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW
	);
}

bool SuggestionList::UpdateSuggestions(const std::wstring& text, HWND hwnd) {
	visibleSuggestions_.clear();
	if (text.length() == 0) return false;
	for (const auto s : allSuggestions) {
		if (s.contains(text)) {
			visibleSuggestions_.push_back(s);
		}
	}
	RECT rc{};
	GetWindowRect(hwnd, &rc);
	SetWindowPos(
		hwnd_,
		nullptr,
		rc.left, rc.bottom, rc.right - rc.left, CalculateHeight(),
		SWP_NOREPOSITION | SWP_NOACTIVATE
	);
	return !visibleSuggestions_.empty();
}

int SuggestionList::CalculateHeight() {
	auto metrics = d2d_.FontMetrics();
	const FLOAT lineHeight = (metrics.ascent + metrics.descent + metrics.lineGap)
		* DpiScaler::ScaleFontSize(9.0) / metrics.designUnitsPerEm + DpiScaler::Scale(verticalPadding);
	return visibleSuggestions_.size() * lineHeight;
}