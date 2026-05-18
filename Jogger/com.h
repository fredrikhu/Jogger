#pragma once
#include <Windows.h>
#include <string>

class ComInitializer {
public:
	ComInitializer();
	~ComInitializer();
	bool IsInitialized();
private:
	bool isInitialized_;
};

bool PickFile(HWND owner, std::wstring& path);