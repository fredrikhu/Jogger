#include <Windows.h>

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
) {
	return MessageBox(NULL, L"hello, world", L"caption", 0);
}