/*
Standard
*/

/*
Include this for all UtilityLibraries libraries.
*/

#pragma once

#include "utility-error.hpp"
#include "utility-filesystem.hpp"
#include "utility-logging.hpp"
#include "utility-string.hpp"
#include "utility-time.hpp"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam);

	//concatenates one map into another, shorthand
	template <class T1, class T2>
	std::map<T1, T2> &concatMap(std::map<T1, T2> &m1, std::map<T1, T2> m2) {
		m1.insert(m2.begin(), m2.end());
		return m1;
	}
	template <class T1, class T2>
	std::map<T1, T2> &concatMap(std::map<T1, T2> &m1, std::map<T1, T2> *m2) {
		m1.insert(m2->begin(), m2->end());
		return m1;
	}

	//creates COLORREF from hex string (e.g. "c8c8e6")
	COLORREF colorFromHex(std::string hex);
}  // namespace Rain
