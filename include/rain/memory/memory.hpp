#include "../platform/.hpp"

namespace Rain {
	// An implementation of GNU memmem, strstr with explicit lengths.
	void *memmem(const void *haystack,
		size_t haystackLen,
		const void *const needle,
		const size_t needleLen) {
#ifdef RAIN_WINDOWS
		for (const char *h = reinterpret_cast<const char *>(haystack);
				 haystackLen >= needleLen;
				 ++h, --haystackLen) {
			if (!memcmp(h, needle, needleLen)) {
				return const_cast<void *>(reinterpret_cast<const void *>(h));
			}
		}
		return NULL;
#else
		return ::memmem(haystack, haystackLen, needle, needleLen);
#endif
	}
}
