/*
Functions to make program logging easier.
*/

#pragma once

#include "windows-lam-include.hpp"
#include "utility-filesystem.hpp"

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <mutex>

namespace Rain {
	// returns a shared mutex which locks cout for functions later
	std::mutex &getCoutMutex() {
		static std::mutex coutMutex;
		return coutMutex;
	}

	// idea to adjust cout for multithreaded applications partially from
	// http://stackoverflow.com/questions/18277304/using-stdcout-in-multiple-threads
	std::ostream &streamOutOne(std::ostream &os) { return os; }
	template <class A0, class... Args>
	std::ostream &streamOutOne(std::ostream &os,
			const A0 &a0,
			const Args &... args) {
		os << a0;
		return streamOutOne(os, args...);
	}
	template <class... Args>
	std::ostream &streamOut(std::ostream &os, const Args &... args) {
		return streamOutOne(os, args...);
	}

	// use this cout function if don't want multiple threads to interrupt each
	// other's output to console TS = thread-safe
	template <class... Args>
	std::ostream &tsCout(const Args &... args) {
		std::lock_guard<std::mutex> m_cout(getCoutMutex());
		return streamOut(std::cout, args...);
	}

	// output to both a logging file and the console; can set logging file via
	// optional parameter truncate the console log if log is too long by defualt,
	// truncate to one line; 0 doesn't truncate
	void outLogStdTrunc(std::string *info,
			int maxLen,
			std::string filePath,
			bool append) {
		static std::string persistentLogFilePath = "";
		if (filePath != "")
			persistentLogFilePath = filePath;
		Rain::printToFile(persistentLogFilePath, info, append);
		if (maxLen != 0)
			Rain::tsCout(info->substr(0, maxLen));
		else
			Rain::tsCout(*info);
	}
	void outLogStdTrunc(std::string info,
			int maxLen,
			std::string filePath,
			bool append) {
		Rain::outLogStdTrunc(&info, maxLen, filePath, append);
	}
	void outLogStd(std::string info, std::string filePath, bool append) {
		Rain::outLogStdTrunc(&info, 0, filePath, append);
	}

	// dumps memory leaks to a file if on debug mode; application must CloseHandle
	// the return HANDLE, unless it's debug mode and the return is NULL
	HANDLE logMemoryLeaks(std::string out_file) {
		if (IsDebuggerPresent()) {
			// Redirect the error stream to a file, only if the program is debugging.
			HANDLE mem_leak = CreateFileA(out_file.c_str(), GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL, NULL);

			// Turn on debugging for memory leaks. This is automatically turned off
			// when the build is Release.
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
			_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_WARN, mem_leak);
			_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_ERROR, mem_leak);
			_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_ASSERT, mem_leak);

			return mem_leak;
		}

		return NULL;
	}
}
