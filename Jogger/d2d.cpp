#include <dwmapi.h>
#include "d2d.h"
#include "DpiScaler.h"
#include <string>

#pragma comment(lib, "Dwmapi.lib")

ComPtr<ID2D1Factory> D2D::factory_;
ComPtr<IDWriteFactory> D2D::writeFactory_;
ComPtr<IDWriteTextFormat> D2D::textFormat_;
DWRITE_FONT_METRICS D2D::fontMetrics_;

bool D2D::CreateD2DFactory() {
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		factory_.GetAddressOf()
	);
	if (FAILED(hr)) return false;
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(writeFactory_.GetAddressOf())
	);
	if (FAILED(hr)) return false;
	hr = writeFactory_->CreateTextFormat(
		L"Segoe UI",                    // font family
		nullptr,                        // font collection
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		// TODO: Scaling
		9.0f, // font size in DIPs
		L"",                            // locale
		textFormat_.GetAddressOf()
	);
	if (FAILED(hr)) return false;
	IDWriteFontCollection* fontCollection;
	hr = writeFactory_->GetSystemFontCollection(&fontCollection);
	if (FAILED(hr)) return false;
	UINT32 nameLength = textFormat_.Get()->GetFontFamilyNameLength();
	std::wstring familyName(nameLength + 1, L'\0');
	hr = textFormat_.Get()->GetFontFamilyName(familyName.data(), familyName.size());
	if (FAILED(hr)) return false;
	UINT32 index;
	BOOL exists;
	hr = fontCollection->FindFamilyName(familyName.c_str(), &index, &exists);
	if (FAILED(hr) || !exists) return false;
	IDWriteFontFamily* fontFamily;
	fontCollection->GetFontFamily(index, &fontFamily);
	if (FAILED(hr)) return false;
	IDWriteFont* font;
	hr = fontFamily->GetFirstMatchingFont(
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		&font
	);
	if (FAILED(hr)) return false;
	ComPtr<IDWriteFontFace> face;
	font->CreateFontFace(&face);
	if (FAILED(hr)) return false;
	face->GetMetrics(&fontMetrics_);

	return true;
}

IDWriteTextFormat* D2D::TextFormat() {
	return textFormat_.Get();
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
	GetClientRect(hwnd, &rc);

	HRESULT hr = factory_->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
		),
		&renderTarget_
	);
	if (FAILED(hr)) return hr;

	// TODO: Recreate when accent color changes (WM_DWMCOLORIZATIONCOLORCHANGED)
	hr = renderTarget_.Get()->CreateSolidColorBrush(
		GetAccentColorF(),
		accentBrush_.GetAddressOf()
	);
	if (FAILED(hr)) {
		renderTarget_.Reset();
		return hr;
	}
	hr = renderTarget_.Get()->CreateSolidColorBrush(
		D2D1::ColorF(0.95f, 0.95f, 0.95f),
		textBrush_.GetAddressOf()
	);
	if (FAILED(hr)) {
		textBrush_.Reset();
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
ComPtr<ID2D1SolidColorBrush> D2D::AccentBrush() {
	return accentBrush_;
}
ComPtr<ID2D1SolidColorBrush> D2D::TextBrush() {
	return textBrush_;
}

void D2D::ResetRenderTarget() {
	renderTarget_.Reset();
	accentBrush_.Reset();
	textBrush_.Reset();
}

DWRITE_FONT_METRICS D2D::FontMetrics() {
	return fontMetrics_;
}