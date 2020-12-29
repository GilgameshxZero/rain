// Abstraction for header lists in Http header blocks.
#pragma once

#include "../../platform.hpp"
#include "../../string.hpp"
#include "../socket.hpp"

#include <map>

namespace Rain::Networking::Http {
	// Case-agnostic key comparison.
	struct HeaderStrCmp {
		bool operator()(const std::string &left, const std::string &right) const {
#ifdef RAIN_WINDOWS
			return _stricmp(left.c_str(), right.c_str()) < 0;
#else
			return strcasecmp(left.c_str(), right.c_str()) < 0;
#endif
		}
	};

	class Header : public std::map<std::string, std::string, HeaderStrCmp> {
		public:
		using std::map<std::string, std::string, HeaderStrCmp>::map;

		bool sendWith(Networking::Socket &socket) {
			for (auto it = this->begin(); it != this->end(); it++) {
				socket.send(it->first);
				socket.send(":");
				socket.send(it->second);
				socket.send("\r\n");
			}
			return false;
		}
	};
}
