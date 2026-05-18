#pragma once
#include "BaseWindow.h"
#include "SuggestionList.h"
#include "d2d.h"
#include <string>
#include <vector>

class MainWindow : public BaseWindow<MainWindow> {
public:
	static bool Register(HINSTANCE hInstance);
	bool Create(HINSTANCE hInstance);
	bool HandleMessage(MSG& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	SuggestionList suggestionList_;

	HINSTANCE hInstance_ = nullptr;
	HWND edit_ = nullptr;
	HWND okButton_ = nullptr;
	HWND browseButton_ = nullptr;

	D2D d2d_;

	HRESULT CreateControls();
	void OnPaint();
	void UpdateSuggestions(const std::wstring& text);
};