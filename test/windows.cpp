// Test compilation of Windows.h include and WinMain entry point and linking of
// Windows libraries.
//
// Windows applications must specify in project settings or pragma comments the
// linker subsystem, or else it will default to CONSOLE.
//
// Specifying in code rather than in the project settings will cause the project
// to build and run fine, but the console to popup during Visual Studio
// debugging (since the linker still thinks the subsystem is CONSOLE).
//
// We specify in code here because specifying in project is inconsistent with
// build.bat.
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#include <rain/gdi-plus.hpp>
#include <rain/literal.hpp>
#include <rain/multithreading.hpp>
#include <rain/time.hpp>
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
	using namespace Rain::Literal;

	char const CAPTION[]{"rain/test/windows"};

	std::thread([&]() {
		std::this_thread::sleep_for(3s);
		PostMessage(FindWindow(NULL, CAPTION), WM_COMMAND, IDOK, 0);
	}).detach();
	return !MessageBox(NULL, "Hello world! Waiting for 3s...", CAPTION, MB_OKCANCEL);
}
#endif
