/*
Standard
*/

#pragma once

#include "network-base.hpp"
#include "network-recv-param.hpp"
#include "network-socket-manager.hpp"
#include "rain-window.hpp"
#include "utility-string.hpp"

#include <functional>
#include <string>
#include <unordered_map>

namespace Rain {
	// test if WSAStartup has beencalled
	bool WSAStarted();

	// both sides
	std::string getClientNumIP(SOCKET &clientSock);

	// send message.length() + ' ' + message over a SocketManager
	void sendBlockMessage(Rain::SocketManager &manager, std::string message);
	void sendBlockMessage(Rain::SocketManager &manager, std::string *message);

	// converts a header map into a HTTP header string
	std::string mapToHTTPHeader(std::map<std::string, std::string> &hMap);
	std::string mapToHTTPHeader(
			std::unordered_map<std::string, std::string> &hMap);

	// create a message queue/window which will respond to messages sent to it
	// RainWindow * which is returned must be freed
	RainWindow *createSendHandler(
			std::unordered_map<UINT, RainWindow::MSGFC> *msgm);

	// converts a GET query string into a map
	std::map<std::string, std::string> getQueryToMap(std::string query);

	// used to send `headed` messages, similar to `block` messages
	void sendHeadedMessage(Rain::SocketManager &manager, std::string message);
	void sendHeadedMessage(Rain::SocketManager &manager, std::string *message);
}

namespace Rain {
	bool WSAStarted() {
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
			return false;
		}
		closesocket(s);
		return true;
	}

	std::string getClientNumIP(SOCKET &clientSock) {
		static struct sockaddr clname;
		static int clnamesize;
		static CHAR clhname[32];

		clnamesize = sizeof(clname);
		getpeername(clientSock, &clname, &clnamesize);
		getnameinfo(&clname, clnamesize, clhname, 32, NULL, 0, NI_NUMERICHOST);

		return std::string(clhname);
	}

	void sendBlockMessage(Rain::SocketManager &manager, std::string message) {
		Rain::sendBlockMessage(manager, &message);
	}
	void sendBlockMessage(Rain::SocketManager &manager, std::string *message) {
		manager.sendRawMessage(Rain::tToStr(message->length()) + " ");
		manager.sendRawMessage(message);
	}

	std::string mapToHTTPHeader(std::map<std::string, std::string> &hMap) {
		std::string message;
		for (auto it = hMap.begin(); it != hMap.end(); it++)
			message += it->first + ": " + it->second + "\n";
		return message + "\n";
	}
	std::string mapToHTTPHeader(
			std::unordered_map<std::string, std::string> &hMap) {
		std::string message;
		for (auto it = hMap.begin(); it != hMap.end(); it++)
			message += it->first + ": " + it->second + "\n";
		return message + "\n";
	}

	RainWindow *createSendHandler(
			std::unordered_map<UINT, RainWindow::MSGFC> *msgm) {
		RainWindow *rw = new RainWindow();
		rw->create(msgm, NULL, 0, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL,
				_T(""), NULL, 0, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL,
				RainWindow::NULLCLASSNAME);

		return rw;
	}

	std::map<std::string, std::string> getQueryToMap(std::string query) {
		std::size_t start, end;
		std::vector<std::string> lines;

		start = 0;
		end = query.find('&');
		while (end != std::string::npos) {
			lines.push_back(query.substr(start, end - start));
			start = end + 1;
			end = query.find('&', start);
		}
		lines.push_back(query.substr(start, end));

		std::map<std::string, std::string> rt;
		for (std::size_t a = 0; a < lines.size(); a++) {
			std::size_t equals = lines[a].find('=');
			rt.insert(std::make_pair(strDecodeURI(lines[a].substr(0, equals)),
					strDecodeURI(lines[a].substr(equals + 1, lines[a].length()))));
		}

		return rt;
	}

	void sendHeadedMessage(Rain::SocketManager &manager, std::string message) {
		sendHeadedMessage(manager, &message);
	}
	void sendHeadedMessage(Rain::SocketManager &manager, std::string *message) {
		std::size_t len = message->length();
		std::string header;
		if (len < 256 * 256) {
			// only need 2-byte header
			header.push_back((len >> 8) & 0xff);
			header.push_back(len & 0xff);
		} else {
			header.push_back('\0');
			header.push_back('\0');
			header.push_back((len >> 24) & 0xff);
			header.push_back((len >> 16) & 0xff);
			header.push_back((len >> 8) & 0xff);
			header.push_back(len & 0xff);
		}

		manager.sendRawMessage(&header);
		manager.sendRawMessage(message);
	}
}
