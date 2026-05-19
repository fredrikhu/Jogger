#pragma once
#include <d2d1.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d2d1.lib")

class D2D {
public:
	static bool CreateD2DFactory();
	HRESULT EnsureRenderTarget(HWND hwnd);
	bool ResizeRenderTarget(UINT width, UINT height);

	

	static ComPtr<ID2D1Factory> Factory();
	ComPtr<ID2D1HwndRenderTarget> RenderTarget();
	ComPtr<ID2D1SolidColorBrush> BackgroundBrush();
private:
	static ComPtr<ID2D1Factory> factory_;
	ComPtr<ID2D1HwndRenderTarget> renderTarget_;
	ComPtr<ID2D1SolidColorBrush> backgroundBrush_;
};
