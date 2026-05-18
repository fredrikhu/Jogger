#include "com.h"

#include <string>
#include <ShObjIdl.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "Ole32.lib")

// TODO: Return bool
HRESULT InitializeCom() {
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	return comResult;
}

void UninitializeCom() {
	CoUninitialize();
}

bool PickFile(HWND owner, std::wstring& path) {
	ComPtr<IFileOpenDialog> dialog;

	HRESULT hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&dialog)
	);
	if (FAILED(hr)) return false;

	hr = dialog->Show(owner);
	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) return false;
	if (FAILED(hr)) return false;

	ComPtr<IShellItem> item;
	hr = dialog->GetResult(&item);
	if (FAILED(hr)) return false;

	PWSTR rawPath = nullptr;
	hr = item->GetDisplayName(SIGDN_FILESYSPATH, &rawPath);
	if (FAILED(hr)) return false;

	path = rawPath;
	CoTaskMemFree(rawPath);

	return true;
}