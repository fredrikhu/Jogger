#pragma once
#include <Windows.h>
#include <commctrl.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

class DpiScaler {
public:
	void Attach(HWND hwnd) {
		hwnd_ = hwnd;
		UINT d = GetDpiForWindow(hwnd_);
		CalculateDpi(d);
		SetWindowSubclass(hwnd_, &DpiScaler::SubclassProc, SubclassId, reinterpret_cast<DWORD_PTR>(this));
	}
	~DpiScaler() {
		if (hwnd_) {
			RemoveWindowSubclass(hwnd_, &DpiScaler::SubclassProc, SubclassId);
			hwnd_ = nullptr;
		}
		if (font_) {
			DeleteObject(font_);
			font_ = nullptr;
		}
	}
	RECT Scale(RECT rect) {
		RECT result = {
			.left = Scale(rect.left),
			.top = Scale(rect.top),
			.right = Scale(rect.right),
			.bottom = Scale(rect.bottom)
		};
		return result;
	}
	LONG Scale(LONG measurement) {
		return static_cast<LONG>(measurement * scaleFactor_);
	}
	void SetScaledFont(HWND hwnd) {
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font_, TRUE);
	}
	FLOAT ScaleFontSize(FLOAT fontSize) {
		return fontSize * dpi_ / TextDpi;
	}
private:
	static constexpr UINT_PTR SubclassId = 1;
	static const FLOAT TextDpi;
	static const FLOAT DefaultDpi;
	HWND hwnd_ = nullptr;
	UINT dpi_;
	float scaleFactor_;
	HFONT font_ = nullptr;

	void CalculateDpi(UINT dpi) {
		if (!hwnd_) return;

		dpi_ = dpi;
		scaleFactor_ = dpi_ / DefaultDpi;
		int fontPointSize_ = -static_cast<int>(ScaleFontSize(9.0f));
		if (font_) {
			DeleteObject(font_);
			font_ = nullptr;
		}
		font_ = CreateFontW(
			fontPointSize_, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
		);
	}

	static LRESULT CALLBACK SubclassProc(
		HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR,
		DWORD_PTR refData
	) {
		auto* self = reinterpret_cast<DpiScaler*>(refData);

		if (uMsg == WM_DPICHANGED)
			self->CalculateDpi(HIWORD(wParam));

		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
};