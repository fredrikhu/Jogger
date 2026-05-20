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

	static ComPtr<ID2D1Factory> Factory();
	static IDWriteTextFormat* TextFormat();
	static DWRITE_FONT_METRICS FontMetrics();
	void ResetRenderTarget();
	ComPtr<ID2D1HwndRenderTarget> RenderTarget();
	ComPtr<ID2D1SolidColorBrush> AccentBrush();
	ComPtr<ID2D1SolidColorBrush> TextBrush();
private:
	static ComPtr<ID2D1Factory> factory_;
	static ComPtr<IDWriteFactory> writeFactory_;
	static ComPtr<IDWriteTextFormat> textFormat_;
	static DWRITE_FONT_METRICS fontMetrics_;
	ComPtr<ID2D1HwndRenderTarget> renderTarget_;
	ComPtr<ID2D1SolidColorBrush> accentBrush_;
	ComPtr<ID2D1SolidColorBrush> textBrush_;
};
