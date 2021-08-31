// Test compilation of Windows.h include and WinMain entry point and linking of
// Windows libraries.
//
// Windows applications must specify in project settings or pragma comments the
// linker subsystem, or else it will default to CONSOLE.
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#include <rain/gdi-plus.hpp>
#include <rain/windows.hpp>

#ifndef RAIN_PLATFORM_WINDOWS
int main() {
	std::cout << "This is a Windows-only test.\n";
	return 0;
}
#else
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow) {
	return !MessageBox(NULL, "Hello world!", "Caption", 0);
}
#endif
