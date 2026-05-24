#pragma once
#include <Windows.h>
#include <commctrl.h>
#include "Rect.h"
#include <d2d1.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

class DpiScaler {
public:
	bool Attach(HWND hwnd) {
		hwnd_ = hwnd;
		if (!CalculateDpi(GetDpiForWindow(hwnd_))) return false;
		return SetWindowSubclass(hwnd_, &DpiScaler::SubclassProc, SubclassId, reinterpret_cast<DWORD_PTR>(this));
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
	Rect Scale(Rect rect) {
		Rect result = {
			.left = Scale(rect.left),
			.top = Scale(rect.top),
			.width = Scale(rect.width),
			.height = Scale(rect.height)
		};
		return result;
	}
	D2D1_RECT_F Descale(D2D1_RECT_F rect) {
		return D2D1_RECT_F {
			.left = Descale(rect.left),
			.top = Descale(rect.top),
			.right = Descale(rect.right),
			.bottom = Descale(rect.bottom)
		};
	}
	LONG Scale(LONG measurement) {
		return static_cast<LONG>(measurement * scaleFactor_);
	}
	FLOAT Scale(FLOAT measurement) {
		return measurement * scaleFactor_;
	}
	FLOAT Descale(FLOAT measurement) {
		return measurement / scaleFactor_;
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
	UINT dpi_{};
	float scaleFactor_{};
	HFONT font_ = nullptr;

	bool CalculateDpi(UINT dpi) {
		if (!hwnd_) return true;
		if (dpi_ == dpi) return true;

		dpi_ = dpi;
		scaleFactor_ = dpi_ / DefaultDpi;
		int fontPointSize_ = -static_cast<int>(ScaleFontSize(9.0f));
		if (font_) {
			auto font = font_;
			font_ = nullptr;

			if (!DeleteObject(font)) return false;
		}

		font_ = CreateFontW(
			fontPointSize_, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
		);
		return font_ != nullptr;
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
		else if (uMsg == WM_SHOWWINDOW && wParam == TRUE)
			self->CalculateDpi(GetDpiForWindow(hwnd));

		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
};