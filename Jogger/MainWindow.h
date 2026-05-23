#pragma once
#include "BaseWindow.h"
#include "SuggestionList.h"
#include "DpiScaler.h"
#include <string>
#include <vector>

class MainWindow : public BaseWindow<MainWindow> {
public:
	static bool RegisterWindowClass(HINSTANCE hInstance);
	bool Create(HINSTANCE hInstance);
	bool HandleMessage(MSG& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	SuggestionList suggestionList_;
	DpiScaler scaler_;

	D2D1::ColorF backgroundColor_ = D2D1::ColorF(0.118f, 0.118f, 0.118f);
	HBRUSH backgroundBrush_;
	HINSTANCE hInstance_ = nullptr;
	HWND edit_ = nullptr;
	HWND okButton_ = nullptr;
	HWND browseButton_ = nullptr;

	HRESULT CreateControls();
	void OnPaint();
	void UpdateSuggestions(const std::wstring& text);
	void ResizeControls();
};