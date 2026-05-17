#include "d2d.h"

bool D2D::CreateD2DFactory() {
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		factory.GetAddressOf()
	);
	return SUCCEEDED(hr);
}

HRESULT D2D::EnsureRenderTarget(HWND hwnd) {
	if (renderTarget.Get()) return S_OK;

	RECT rc;
	GetWindowRect(hwnd, &rc);

	HRESULT hr = factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
		),
		&renderTarget
	);
	if (FAILED(hr)) return hr;

	hr = renderTarget.Get()->CreateSolidColorBrush(
		D2D1::ColorF(0xFFFFFF),
		backgroundBrush.GetAddressOf()
	);
	if (FAILED(hr)) {
		renderTarget.Reset();
		return hr;
	}

	return S_OK;
}

bool D2D::ResizeRenderTarget(UINT width, UINT height) {
	HRESULT hr = renderTarget.Get()->Resize(D2D1::SizeU(width, height));
	return SUCCEEDED(hr);
}