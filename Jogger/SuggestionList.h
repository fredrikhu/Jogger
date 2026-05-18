#pragma once
#include "BaseWindow.h"
#include <vector>
#include <string>

class SuggestionList : public BaseWindow<SuggestionList> {
public:
	static bool Register(HINSTANCE hInstance);
	bool Create(HINSTANCE hInstance, HWND owner);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ShowBelow(HWND hwnd);
	void PositionBelow(HWND hwnd);
	bool UpdateSuggestions(const std::wstring& text);
private:
	HINSTANCE hInstance_ = nullptr;
	std::vector<std::wstring> visibleSuggestions_;
};