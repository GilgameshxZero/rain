#pragma once

#include "../platform.hpp"
#include "exception.hpp"
#include "windows.hpp"

#include <iostream>
#include <string>
#include <vector>

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	// Based on
	// https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application.
	// Retrieves theme mode for Windows. May throw on earlier versions if registry
	// key does not exist.
	inline bool isLightTheme() {
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
