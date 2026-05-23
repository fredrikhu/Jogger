#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite.lib")


class D2D {
public:
	static bool CreateD2DFactory();
	HRESULT EnsureRenderTarget(HWND hwnd);
	void ResizeRenderTarget(UINT width, UINT height);
	bool Attach(HWND hwnd);
	~D2D();

	static ComPtr<ID2D1Factory> Factory();
	IDWriteTextFormat* TextFormat();
	DWRITE_FONT_METRICS FontMetrics();
	void ResetRenderTarget();
	ComPtr<ID2D1HwndRenderTarget> RenderTarget();
	ComPtr<ID2D1SolidColorBrush> AccentBrush();
	ComPtr<ID2D1SolidColorBrush> TextBrush();
private:
	static ComPtr<ID2D1Factory> factory_;
	static ComPtr<IDWriteFactory> writeFactory_;
	HWND hwnd_ = nullptr;
	ComPtr<IDWriteTextFormat> textFormat_;
	DWRITE_FONT_METRICS fontMetrics_;
	ComPtr<ID2D1HwndRenderTarget> renderTarget_;
	ComPtr<ID2D1SolidColorBrush> accentBrush_;
	ComPtr<ID2D1SolidColorBrush> textBrush_;
	static constexpr UINT_PTR SubclassId = 1;
	UINT dpi_ = 0;

	void CalculateDpi(UINT dpi);
	static LRESULT CALLBACK SubclassProc(
		HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR,
		DWORD_PTR refData
	);
};
