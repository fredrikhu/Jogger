#pragma once
#include "BaseWindow.h"

class SuggestionList : public BaseWindow<SuggestionList> {
public:
	static bool Register(HINSTANCE hInstance);
	bool Create(HINSTANCE hInstance, HWND owner);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HINSTANCE hInstance_ = nullptr;
};