#pragma once
#include <d2d1.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d2d1.lib")

extern ComPtr<ID2D1Factory> factory;
extern ComPtr<ID2D1HwndRenderTarget> renderTarget;
extern ComPtr<ID2D1SolidColorBrush> backgroundBrush;

bool CreateD2DFactory();
HRESULT EnsureRenderTarget(HWND hwnd);
bool ResizeRenderTarget(UINT width, UINT height);