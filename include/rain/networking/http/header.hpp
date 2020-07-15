// Abstraction for header lists in Http header blocks.
#pragma once

#include "../../platform.hpp"
#include "../../string.hpp"

#include <map>

namespace Rain::Networking::Http {
	struct HeaderStrCmp {
		bool operator()(const std::string &left, const std::string &right) const {
#ifdef RAIN_WINDOWS
			return _stricmp(left.c_str(), right.c_str()) < 0;
#else
			return strcasecmp(left.c_str(), right.c_str()) < 0;
#endif
		}
	};
	typedef std::map<std::string, std::string, HeaderStrCmp> Header;
}
