/*
	thread-safe versatile class that can input from stdin, stdout, strings and
	sockets, and output to stdout and files one LogStream can IO from multiple
	sources, but its log stream will be the same to all outputs; if multiple
	logs are needed, use multiple LogStreams
*/

#pragma once

#include "network-socket-manager.hpp"
#include "utility-filesystem.hpp"
#include "utility-logging.hpp"
#include "utility-time.hpp"

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <mutex>

namespace Rain {
	class LogStream {
	 public:
		LogStream();
		~LogStream();

		// enable/disable logging of socket communications
		void setSocketSrc(Rain::SocketManager *nsm, bool enable);

		// enable/disable a standard handle logging source
		// use STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, or STD_ERROR_HANDLE
		bool setStdHandleSrc(DWORD stdHandle, bool enable);

		// output a string to logging outputs
		// called by SocketManagers when they need to log anything
		void logString(std::string *s);
		void logString(std::string s);

		// enable/disable log output file
		void setFileDst(std::string path, bool enable);

		// enable/disable stdout logging destination
		// can also set a rule to truncate each log to a certian length; 0 is don't
		// truncate don't capture what we log to stdout doesn't work with
		// STD_INPUT_HANDLE for now, and STD_ERROR_HANDLE is untested
		void setStdoutDst(bool enable, std::size_t len = 0);

	 private:
		struct StdSrcRedirectThreadParam {
			// pipe to read from and write to, in addition to logging the information
			// in the process
			HANDLE rd, wr;

			// whether thread should immediately exit or not
			bool running;

			// object used to log
			LogStream *logger;
		};

		struct StdSrcInfo {
			// read and write of replacement pipe of standard source
			HANDLE repPipeRd, repPipeWr;

			// os handles of original standard source
			int oshOrigStdSrc;

			// os handle for repPipeWr
			int oshRepPipeWr;

			// parameter that is passed to the thread
			StdSrcRedirectThreadParam thParam;

			// handle to the thread
			HANDLE hThread;
			std::thread hStdThread;
		};

		std::set<std::string> fileDst;
		bool outputStdout;
		std::size_t stdoutTrunc;

		// map of all std handles to input from
		std::map<DWORD, StdSrcInfo *> stdSrcMap;

		// thread which captures information from a pipe, and redirects to another
		// pipe, as well as the logger parameter is a pointer to a tuple: (rd_pipe,
		// wr_pipe, LogStream *, bool *) terminates when rd_pipe is closed
		static DWORD WINAPI stdSrcRedirectThread(LPVOID lpParameter);
	};

	LogStream::LogStream() {
		this->outputStdout = false;
		this->stdoutTrunc = 0;
	}
	LogStream::~LogStream() {
		// terminate any threads
		for (auto it : this->stdSrcMap) {
			this->setStdHandleSrc(it.first, false);
		}
	}
	void LogStream::setSocketSrc(Rain::SocketManager *nsm, bool enable) {
		nsm->setLogging(enable ? reinterpret_cast<void *>(this) : NULL);
	}
	bool LogStream::setStdHandleSrc(DWORD stdHandle, bool enable) {
		bool ret = this->stdSrcMap.find(stdHandle) != this->stdSrcMap.end();
		FILE *stdFilePtr;
		if (stdHandle == STD_OUTPUT_HANDLE)
			stdFilePtr = stdout;
		else if (stdHandle == STD_INPUT_HANDLE)
			stdFilePtr = stdin;
		else
			stdFilePtr = stderr;
		if (enable && !ret) {
			// redirect stdin to a pipe, then from that pipe to the original stdin
			// pipe
			HANDLE rd, wr;
			CreatePipe(&rd, &wr, NULL, 0);
			int oshOrigStdSrc = _dup(_fileno(stdFilePtr)),
					oshRepPipeWr = _open_osfhandle(reinterpret_cast<intptr_t>(wr), 0);
			HANDLE hOrig = reinterpret_cast<HANDLE>(_get_osfhandle(oshOrigStdSrc));
			_dup2(oshRepPipeWr, _fileno(stdFilePtr));

			// save handles to a map, to access later when the stdsrc is disabled
			StdSrcInfo &ssi =
					*this->stdSrcMap.insert(std::make_pair(stdHandle, new StdSrcInfo()))
							 .first->second;
			ssi.thParam.logger = this;
			ssi.thParam.rd = rd;
			ssi.thParam.wr = hOrig;
			ssi.thParam.running = true;

			ssi.oshOrigStdSrc = oshOrigStdSrc;
			ssi.oshRepPipeWr = oshRepPipeWr;
			ssi.repPipeRd = rd;
			ssi.repPipeWr = wr;
			ssi.hStdThread = std::thread(LogStream::stdSrcRedirectThread,
					reinterpret_cast<LPVOID>(&ssi.thParam));
			ssi.hThread = ssi.hStdThread.native_handle();
		} else if (!enable && ret) {
			fflush(stdFilePtr);

			// get params
			StdSrcInfo &ssi = *this->stdSrcMap.find(stdHandle)->second;

			// stop logging this pipe and terminate corresponding thread
			ssi.thParam.running = false;
			CancelSynchronousIo(ssi.hThread);
			WaitForSingleObject(ssi.hThread, INFINITE);
			ssi.hStdThread.join();
			_close(ssi.oshRepPipeWr);	// calls CloseHandle on ssi.repPipeWr
			CloseHandle(ssi.repPipeRd);

			// reset std handles and free memory
			this->stdSrcMap.erase(
					stdHandle);	// erase here, so that we can log everything in stdout if
											 // we are logging to stdout as well
			_dup2(ssi.oshOrigStdSrc, _fileno(stdFilePtr));
			delete &ssi;
		}
		return ret;
	}
	void LogStream::logString(std::string *s) {
		// logging will be thread-safe
		static std::mutex logMutex;

		static std::string header;

		logMutex.lock();

		header = Rain::getTime() + " " + Rain::tToStr(s->length()) + Rain::CRLF;
		if (this->outputStdout) {
			// if STD_OUTPUT_HANDLE is being logged, then temporarily replace with
			// original output handles while outputting log, so that we don't log
			// indefinitely
			auto itMap = this->stdSrcMap.find(STD_OUTPUT_HANDLE);
			if (itMap != this->stdSrcMap.end()) {
				std::cout.flush();
				_dup2(itMap->second->oshOrigStdSrc, _fileno(stdout));
			}
			Rain::tsCout(header,
					s->substr(0,
							this->stdoutTrunc == 0 ? std::string::npos : this->stdoutTrunc),
					CRLF, CRLF);
			if (itMap != this->stdSrcMap.end()) {
				std::cout.flush();
				_dup2(itMap->second->oshRepPipeWr, _fileno(stdout));
			}
		}
		for (std::string file : this->fileDst) {
			Rain::printToFile(file, &header, true);
			Rain::printToFile(file, s, true);
			Rain::printToFile(file, CRLF + CRLF, true);
		}

		logMutex.unlock();
	}
	void LogStream::logString(std::string s) { this->logString(&s); }
	void LogStream::setFileDst(std::string path, bool enable) {
		if (enable)
			this->fileDst.insert(path);
		else
			this->fileDst.erase(path);
	}
	void LogStream::setStdoutDst(bool enable, std::size_t len) {
		this->outputStdout = enable;
		this->stdoutTrunc = len;
	}
	DWORD WINAPI LogStream::stdSrcRedirectThread(LPVOID lpParameter) {
		static const std::size_t bufSize = 65536;
		StdSrcRedirectThreadParam &param =
				*reinterpret_cast<StdSrcRedirectThreadParam *>(lpParameter);

		DWORD dwRead, dwWritten;
		CHAR chBuf[bufSize];
		BOOL bSuccess = FALSE;
		std::string message;

		while (param.running) {
			bSuccess = ReadFile(param.rd, chBuf, bufSize, &dwRead,
					NULL);	// blocks until cancelled or read
			if (!bSuccess || dwRead == 0)
				break;
			param.logger->logString(std::string(
					chBuf, dwRead));	// log here before console in case of error
			bSuccess = WriteFile(param.wr, chBuf, dwRead, &dwWritten, NULL);
			if (!bSuccess)
				break;	// unexpected error
		}

		// if we're here, read what's remaining in the pipe if possible before
		// exiting
		DWORD bytesAvail;
		PeekNamedPipe(param.rd, NULL, 0, NULL, &bytesAvail, NULL);
		while (bytesAvail > 0) {	// the same procedure as the rest of the loop
			ReadFile(param.rd, chBuf, bufSize, &dwRead,
					NULL);	// don't check for error, since synchronious io is already
									// cancelled, so this will probably error
			param.logger->logString(std::string(chBuf, dwRead));
			if (!WriteFile(param.wr, chBuf, dwRead, &dwWritten, NULL))
				break;
			PeekNamedPipe(param.rd, NULL, 0, NULL, &bytesAvail, NULL);
		}

		return 0;
	}
}
