#include "utility-error.hpp"

namespace Rain {
	int reportError(int code, std::string desc) {
		if (desc != "")
			std::cerr << "[";
		std::cerr << code;
		if (desc != "")
			std::cerr << "] " << desc;
		std::cerr << Rain::CRLF;
		return code;
	}
	int errorAndCout(int code, std::string desc, std::string endl) {
		tsCout("[", code, "] ", desc, endl);
		return reportError(code, desc);
	}

	std::pair<std::streambuf *, std::ofstream *> redirectCerrFile(
		std::string filename,
		bool append) {
		std::ofstream *cerrfile = new std::ofstream();
		std::ios_base::openmode openmode = std::ios_base::out | std::ios_base::binary;
		if (append) {
			openmode |= std::ios_base::app;
		}
		cerrfile->open(filename, openmode);
		return std::make_pair(std::cerr.rdbuf(cerrfile->rdbuf()), cerrfile);
	}
}
