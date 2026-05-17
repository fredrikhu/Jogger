#pragma once
#include "BaseWindow.h"
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
	HINSTANCE hInstance_ = nullptr;
	HWND edit = nullptr;
	HWND okButton = nullptr;
	HWND browseButton = nullptr;
	HWND suggestionList = nullptr;
	std::vector<std::wstring> visibleSuggestions_;

	D2D d2d_;

	HRESULT CreateControls();
	void OnPaint();
	void UpdateSuggestions(const std::wstring& text);
};