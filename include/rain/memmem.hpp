#include "platform.hpp"

#include <cstring>

namespace Rain {
	// An implementation of GNU memmem, strstr with explicit lengths.
	inline void *memmem(const void *haystack,
		std::size_t haystackLen,
		const void *const needle,
		const std::size_t needleLen) {
		for (const char *h = reinterpret_cast<const char *>(haystack);
				 haystackLen >= needleLen;
				 ++h, --haystackLen) {
			if (!memcmp(h, needle, needleLen)) {
				return const_cast<void *>(reinterpret_cast<const void *>(h));
			}
		}
		return NULL;
	}
}
