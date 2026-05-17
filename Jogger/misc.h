#pragma once
#include <Windows.h>

namespace {
	HMENU ControlId(int id) {
		return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
	}

	std::wstring GetText(HWND hwnd) {
		const int length = GetWindowTextLengthW(hwnd);

		std::wstring text(length + 1, L'\0');
		const int copied = GetWindowTextW(hwnd, text.data(), static_cast<int>(text.size()));

		text.resize(copied);
		return text;
	}

	bool LaunchWithShell(
		HWND owner,
		const std::wstring& command
	) {
		SHELLEXECUTEINFOW info{
			.cbSize = sizeof(info),
			.hwnd = owner,
			.lpVerb = L"open",
			.lpFile = command.c_str(),
			.nShow = SW_SHOWNORMAL,
		};
		return ShellExecuteExW(&info) != FALSE;
	}
}