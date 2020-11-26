// SMTP client implementation.
#pragma once

#include "../../platform.hpp"
#include "socket.hpp"

#ifdef RAIN_WINDOWS
#include <WinDNS.h>
#pragma comment(lib, "Dnsapi.lib")
#else
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/types.h>
#endif

#include <iostream>

namespace Rain::Networking::Smtp {
	class Client : protected Smtp::Socket {
		public:
		Client(std::size_t bufSz = 16384) : Smtp::Socket(), bufSz(bufSz) {
			this->buffer = new char[bufSz];
		}
		~Client() { delete[] this->buffer; }

		std::size_t recvTimeoutMs = 5000;

		int connectDomain(const std::string &domain) {
			// Get SMTP server addresses.
			std::vector<std::string> servers;

#ifdef RAIN_WINDOWS
			DNS_RECORD *dnsRecord;
			if (!DnsQuery(domain.c_str(),
						DNS_TYPE_MX,
						DNS_QUERY_STANDARD,
						NULL,
						&dnsRecord,
						NULL)) {
				DNS_RECORD *curDNSR = dnsRecord;
				while (curDNSR != NULL && curDNSR->wType == DNS_TYPE_MX) {
					servers.push_back(curDNSR->Data.MX.pNameExchange);
					curDNSR = curDNSR->pNext;
				}
				DnsRecordListFree(dnsRecord, DnsFreeRecordList);
			} else {
				return -1;
			}
#else
			u_char res_res[NS_PACKETSZ];
			int len;
			if ((len = res_query(
						(domain + ".").c_str(), C_IN, T_MX, res_res, sizeof(res_res))) < 0) {
				return -3;
			}
			ns_msg handle;
			if (ns_initparse(res_res, len, &handle) < 0) {
				return -4;
			}
			len = ns_msg_count(handle, ns_s_an);
			if (len < 0) {
				return -5;
			}
			ns_rr rr;
			char dispbuf[4096];
			for (int ns_index = 0; ns_index < len; ns_index++) {
				if (ns_parserr(&handle, ns_s_an, ns_index, &rr)) {
					continue;
				}
				ns_sprintrr(&handle, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
				if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == ns_t_mx) {
					char mxname[MAXDNAME];
					dn_expand(ns_msg_base(handle),
						ns_msg_base(handle) + ns_msg_size(handle),
						ns_rr_rdata(rr) + NS_INT16SZ,
						mxname,
						sizeof(mxname));
					servers.push_back(strdup(mxname));
				}
			}
#endif

			bool connected = false;
			this->create();
			for (auto server : servers) {
				try {
					this->connect(Host(server.c_str(), 25));
					connected = true;
					break;
				} catch (...) {
					// Do nothing; just try the next one.
				}
			}
			Response res;
			parseResponse(&res);
			connected = connected && res.code == 220;

			return connected ? 0 : -2;
		}
		int sendEmail(const std::string &ehloParam,
			const std::string &from,
			const std::string &to,
			const std::string &data) {
			Request req;
			Response res;
			req.verb = "EHLO";
			req.parameter = ehloParam;
			this->send(&req);
			parseResponse(&res);
			if (res.code != 250) {
				return -1;
			}

			req.verb = "MAIL";
			req.parameter = "FROM:<" + from + ">";
			this->send(&req);
			parseResponse(&res);
			if (res.code != 250) {
				return -2;
			}

			req.verb = "RCPT";
			req.parameter = "TO:<" + to + ">";
			this->send(&req);
			parseResponse(&res);
			if (res.code != 250) {
				return -3;
			}

			req.verb = "DATA";
			req.parameter = "";
			this->send(&req);
			parseResponse(&res);
			if (res.code != 354) {
				return -4;
			}

			this->send(data);
			parseResponse(&res);
			if (res.code != 250) {
				return -5;
			}

			req.verb = "QUIT";
			req.parameter = "";
			this->send(&req);
			parseResponse(&res);
			if (res.code != 221) {
				return -6;
			}

			return 0;
		}
		int parseResponse(Response *res) {
			static const char *CRLF = "\r\n";
			static const std::size_t PART_MATCH_CRLF[] = {
				(std::numeric_limits<std::size_t>::max)(), 0, 0};

			// Parse the entire header.
			// State of newline search.
			std::size_t kmpCand = 0;
			char *curRecv = this->buffer,	 // Last search position.
				*curParse = this->buffer;

			// Keep on calling recv until we find get the full response.
			res->code = 0;
			while (true) {
				std::size_t bufRemaining = this->bufSz - (curRecv - this->buffer);
				if (bufRemaining == 0) {
					return -2;
				}
				std::size_t recvLen = this->recv(
					curRecv, bufRemaining, RecvFlag::NONE, this->recvTimeoutMs);
				if (recvLen == 0) {
					return -1;
				}

				// Look to parse an additional line.
				char *newline = Algorithm::cStrSearchKMP(
					curRecv, recvLen, CRLF, 2, PART_MATCH_CRLF, &kmpCand);
				if (newline != NULL) {
					// Found full line since curParse.
					bool isLastLine = curParse[3] == ' ';
					curParse[3] = '\0';
					*newline = '\0';
					if (res->code == 0) {
						// First line.
						res->code = std::strtoll(curParse, NULL, 10);
						res->parameter = std::string(curParse + 4);
					} else {
						// Extension line.
						res->extensions.emplace_back(std::vector<std::string>());
						for (char *space = curParse + 4, *prevSpace = curParse + 4;
								 space != newline + 1;
								 space++) {
							if (*space == ' ' || *space == '\0') {
								*space = '\0';
								res->extensions.back().emplace_back(std::string(prevSpace));
								prevSpace = space + 1;
							}
						}
					}

					if (isLastLine) {
						break;
					}
				}
				curRecv += recvLen;
			}

			return 0;
		}

		private:
		std::size_t bufSz;
		char *buffer;
	};
}
