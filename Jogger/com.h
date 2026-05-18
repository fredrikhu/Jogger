#pragma once
#include <Windows.h>
#include <string>

HRESULT InitializeCom();
void UninitializeCom();
bool PickFile(HWND owner, std::wstring& path);