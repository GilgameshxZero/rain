#pragma once

#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	// Errors in the message loop throw with NONE.
	inline void runMessageLoop(HWND hWnd = NULL) {
		BOOL bRet;
		MSG msg;
		while ((bRet = GetMessage(&msg, hWnd, 0, 0)) != 0) {
			if (bRet == -1) {
				throw Exception(Error::NONE);
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	// Retrieves theme mode for Windows. May throw on earlier versions if registry
	// key does not exist.
	inline bool isLightTheme() {
		// Based on
		// https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application.
		static DWORD buffer, cbData{sizeof(buffer)};
		validateSystemCallDirect(RegGetValue(
			HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			"AppsUseLightTheme",
			RRF_RT_REG_DWORD,
			nullptr,
			&buffer,
			&cbData));
		return buffer > 0;
	}
}

#endif
