/*
Error-reporting.
*/

#pragma once

#include "utility-logging.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace Rain {
	int reportError(int code, std::string desc = "");
	int errorAndCout(int code, std::string desc = "", std::string endl = CRLF);
	std::pair<std::streambuf *, std::ofstream *> redirectCerrFile(
			std::string filename,
			bool append = false);
}
