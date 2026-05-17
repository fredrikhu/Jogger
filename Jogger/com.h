#pragma once
#include <Windows.h>

HRESULT InitializeCom();
void UninitializeCom();
bool PickFile(HWND owner, std::wstring& path);