#pragma once
#include <Windows.h>
#include <cassert>
#include "d2d.h"

class PaintSession {
public:
	PaintSession(HWND hwnd, D2D& d2d) : hwnd_(hwnd), d2d_(d2d) {
		::BeginPaint(hwnd_, &ps);
		if (FAILED(d2d_.EnsureRenderTarget(hwnd_))) return;
		renderTarget_ = d2d_.RenderTarget().Get();
		renderTarget_->BeginDraw();
	}
	~PaintSession() {
		if (renderTarget_) {
			HRESULT hr = renderTarget_->EndDraw();
			if (hr == D2DERR_RECREATE_TARGET) {
				d2d_.ResetRenderTarget();
				InvalidateRect(hwnd_, nullptr, FALSE);
			}
		}
		::EndPaint(hwnd_, &ps);
	}
	PaintSession(const PaintSession&) = delete;
	PaintSession& operator=(const PaintSession&) = delete;
	ID2D1HwndRenderTarget* RenderTarget() {
		return renderTarget_;
	}
private:
	PAINTSTRUCT ps;
	HWND hwnd_;
	ID2D1HwndRenderTarget* renderTarget_ = nullptr;
	D2D& d2d_;
};

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
	PaintSession BeginPaint() {
		return PaintSession(hwnd_, d2d_);
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

		if (!self) {
			self = reinterpret_cast<Derived*>(
				GetWindowLongPtrW(hwnd, GWLP_USERDATA)
			);
		}

		if (msg == WM_DPICHANGED) {
			// There is an edge case where DPI can change and the size does
			// not. We will not handle this since it's to rare of an occurence.
			const RECT* rc = reinterpret_cast<const RECT*>(lParam);
			SetWindowPos(
				hwnd,
				nullptr,
				rc->left, rc->top,
				rc->right - rc->left, rc->bottom - rc->top,
				SWP_NOZORDER | SWP_NOACTIVATE
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
