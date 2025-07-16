// On Windows, WSAStartup must be called before any socket functionality is
// used. This allocates an automatic-duration global variable to handle that.
#pragma once

#include "exception.hpp"

namespace Rain::Networking {
#ifdef RAIN_PLATFORM_WINDOWS
	class AutoWsaManager {
		private:
		// Calls WSAStartup on construct, WSACleanup on destruct.
		class _ {
			public:
			WSADATA wsaData;

			_() {
				int startupReturn{WSAStartup(MAKEWORD(2, 2), &wsaData)};
				if (startupReturn != 0) {
					// An immediate failure can throws an error code from the WSA set of
					// codes.
					throw Networking::Exception(
						static_cast<Networking::Error>(startupReturn));
				}
			};
			~_() { validateSystemCall(WSACleanup()); }
		};

		// Constructed during global initialization (before main), destructed after
		// main.
		static inline _ invoker;

		public:
		// Get WSADATA from WSAStartup.
		static WSADATA &wsaData() { return AutoWsaManager::invoker.wsaData; }
	};
#endif
}
