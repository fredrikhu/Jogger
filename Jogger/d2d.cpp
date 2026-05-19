#include "d2d.h"
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

ComPtr<ID2D1Factory> D2D::factory_;

bool D2D::CreateD2DFactory() {
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		factory_.GetAddressOf()
	);
	return SUCCEEDED(hr);
}

D2D1_COLOR_F GetAccentColorF() {
	DWORD color = 0;
	BOOL opaqueBlend = FALSE;

	if (FAILED(DwmGetColorizationColor(&color, &opaqueBlend))) {
		return D2D1::ColorF(0.0f, 0.47f, 0.84f);
	}

	return D2D1::ColorF(
		((color >> 16) & 0xff) / 255.0f,
		((color >> 8) & 0xff) / 255.0f,
		(color & 0xff) / 255.0f,
		((color >> 24) & 0xff) / 255.0f
	);
}

HRESULT D2D::EnsureRenderTarget(HWND hwnd) {
	if (renderTarget_.Get()) return S_OK;

	RECT rc;
	GetWindowRect(hwnd, &rc);

	HRESULT hr = factory_->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
		),
		&renderTarget_
	);
	if (FAILED(hr)) return hr;

	hr = renderTarget_.Get()->CreateSolidColorBrush(
		GetAccentColorF(),
		backgroundBrush_.GetAddressOf()
	);
	if (FAILED(hr)) {
		renderTarget_.Reset();
		return hr;
	}

	return S_OK;
}

void D2D::ResizeRenderTarget(UINT width, UINT height) {
	if (!renderTarget_.Get()) return;

	renderTarget_.Get()->Resize(D2D1::SizeU(width, height));
}

ComPtr<ID2D1Factory> D2D::Factory() {
	return factory_;
}
ComPtr<ID2D1HwndRenderTarget> D2D::RenderTarget() {
	return renderTarget_;
}
ComPtr<ID2D1SolidColorBrush> D2D::BackgroundBrush() {
	return backgroundBrush_;
}

void D2D::ResetRenderTarget() {
	renderTarget_.Reset();
	backgroundBrush_.Reset();
}