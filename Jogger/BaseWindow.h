#pragma once
#include <Windows.h>
#include <cassert>
#include "d2d.h"

template <typename Derived>
class BaseWindow {
public:
	HWND Window() const { return hwnd_; }
	void Show(int showCommand) {
		assert(hwnd_);
		if (!hwnd_)  return;
		ShowWindow(hwnd_, showCommand);
	}
	void Hide() {
		assert(hwnd_);
		if (!hwnd_)  return;
		ShowWindow(hwnd_, SW_HIDE);
	}
	void Destroy() {
		if (!hwnd_)  return;
		DestroyWindow(hwnd_);
		hwnd_ = nullptr;
	}

	static LRESULT CALLBACK WindowProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	) {
		Derived* self = nullptr;
		if (msg == WM_NCCREATE) {
			auto create = reinterpret_cast<CREATESTRUCTW*>(lParam);
			self = static_cast<Derived*>(create->lpCreateParams);

			SetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(self)
			);

			self->hwnd_ = hwnd;
		}
		else {
			self = reinterpret_cast<Derived*>(
				GetWindowLongPtrW(hwnd, GWLP_USERDATA)
			);
		}

		if (self)
			return self->HandleMessage(msg, wParam, lParam);
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
protected:
	HWND hwnd_ = nullptr;
	D2D d2d_;
};

class PaintSession {
public:
	PaintSession(HWND hwnd, D2D& d2d) : hwnd_(hwnd), d2d_(d2d) {
		BeginPaint(hwnd_, &ps);
		if (FAILED(d2d_.EnsureRenderTarget(hwnd_))) return;
		renderTarget_ = d2d_.RenderTarget().Get();
		renderTarget_->BeginDraw();
	}
	~PaintSession() {
		EndPaint(hwnd_, &ps);
		HRESULT hr = renderTarget_->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET) {
			d2d_.RenderTarget().Reset();
			d2d_.BackgroundBrush().Reset();
		}
	}
	ID2D1HwndRenderTarget* RenderTarget() {
		return renderTarget_;
	}
	bool Failed() {
		return renderTarget_ == nullptr;
	}
private:
	PAINTSTRUCT ps;
	HWND hwnd_;
	ID2D1HwndRenderTarget* renderTarget_ = nullptr;
	D2D& d2d_;
};