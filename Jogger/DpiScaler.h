#pragma once
#include <Windows.h>

class DpiScaler {
public:
	static void Initialize() {
		dpi_ = GetDpiForSystem();
		scaleFactor_ = dpi_ / (float)DefaultDpi;
		int fontPointSize_ = -static_cast<int>(9 * dpi_ / 72.0f);
		font_ = CreateFontW(
			fontPointSize_, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
		);
	}
	static RECT Scale(RECT rect) {
		RECT result = {
			.left = Scale(rect.left),
			.top = Scale(rect.top),
			.right = Scale(rect.right),
			.bottom = Scale(rect.bottom)
		};
		return result;
	}
	static LONG Scale(LONG measurement) {
		return static_cast<LONG>(measurement * scaleFactor_);
	}
	static void SetScaledFont(HWND hwnd) {
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font_, TRUE);
	}
	static float ScaleFontSize(float fontSize) {
		return fontSize * dpi_ / 72.0f;
	}
private:
	static const UINT DefaultDpi = 96;
	static UINT dpi_;
	static float scaleFactor_;
	static HFONT font_;
};