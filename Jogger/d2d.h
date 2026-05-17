#pragma once
#include <d2d1.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d2d1.lib")

class D2D {
public:
	bool CreateD2DFactory();
	HRESULT EnsureRenderTarget(HWND hwnd);
	bool ResizeRenderTarget(UINT width, UINT height);

	ComPtr<ID2D1Factory> Factory();
	ComPtr<ID2D1HwndRenderTarget> RenderTarget();
	ComPtr<ID2D1SolidColorBrush> BackgroundBrush();
private:
	ComPtr<ID2D1Factory> factory;
	ComPtr<ID2D1HwndRenderTarget> renderTarget;
	ComPtr<ID2D1SolidColorBrush> backgroundBrush;
};
