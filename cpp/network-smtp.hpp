/*
Standard
*/

#pragma once

#include "utility-string.hpp"

namespace Rain {
	// retreive the status code from a message, such as 250 or 221
	// returns -1 for parsing error
	int getSMTPStatus(std::string *message);
	int getSMTPStatus(std::string message);

	// gets email parts from a string
	std::string getEmailDomain(std::string email);
	std::string getEmailUser(std::string email);
}

namespace Rain {
	int getSMTPStatus(std::string *message) {
		// get the last line
		std::size_t lastCLRF = message->rfind(CRLF),
								stlCLRF = message->rfind(CRLF, lastCLRF - 1);
		if (lastCLRF == std::string::npos)
			return -1;
		std::string lastLine =
				message->substr((stlCLRF == std::string::npos ? 0 : stlCLRF), lastCLRF);

		// status code is anything up to the first space
		std::size_t firstSpace = lastLine.find(" ");
		if (firstSpace == std::string::npos)
			return -1;
		return Rain::strToT<int>(lastLine.substr(0, firstSpace));
	}
	int getSMTPStatus(std::string message) { return getSMTPStatus(&message); }

	std::string getEmailDomain(std::string email) {
		return email.substr(email.find('@') + 1, email.length());
	}
	std::string getEmailUser(std::string email) {
		return email.substr(0, email.find('@'));
	}
}
