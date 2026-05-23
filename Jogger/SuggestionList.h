#pragma once
#include "BaseWindow.h"
#include "DpiScaler.h"
#include <vector>
#include <string>

class SuggestionList : public BaseWindow<SuggestionList> {
public:
	static bool RegisterWindowClass(HINSTANCE hInstance);
	bool Create(HINSTANCE hInstance, HWND owner);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ShowBelow(HWND hwnd);
	void PositionBelow(HWND hwnd);
	bool UpdateSuggestions(const std::wstring& text, HWND hwnd);
private:
	void OnPaint();
	int CalculateHeight();
	static const FLOAT verticalPadding;

	DpiScaler scaler_;
	HINSTANCE hInstance_ = nullptr;
	std::vector<std::wstring> visibleSuggestions_;
};