/*
Supplemental GDI+ libraries on Windows.
*/

#pragma once

#include "gdi-plus-include.hpp"
#include "platform.hpp"

#ifdef RAIN_WINDOWS

namespace Rain {
	int getEncoderClsid(const WCHAR *format, CLSID *pClsid);
}

#endif
