// SMTP client implementation.
#pragma once

#ifdef UNICODE
#undef UNICODE
#endif

#include "../../platform.hpp"
#include "../request-response/client.hpp"
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

// Compatibility layer for outdated arpa/nameser.h implementations on MacOS.
// TODO: Make this robust to other options.
#ifndef MAXDNAME
#define MAXDNAME NI_MAXHOST
#endif

#ifndef C_IN
#define C_IN 1
#endif

#ifndef T_MX
#define T_MX 15
#endif
#endif

namespace Rain::Networking::Smtp {
	class Client : virtual protected Socket,
								 public RequestResponse::Client<Request, Response> {
		public:
		using RequestResponse::Client<Request, Response>::Client;

		using RequestResponse::Client<Request, Response>::close;
		using RequestResponse::Client<Request, Response>::connect;
		using RequestResponse::Client<Request, Response>::getFamily;
		using RequestResponse::Client<Request, Response>::getNativeSocket;
		using RequestResponse::Client<Request, Response>::getProtocol;
		using RequestResponse::Client<Request, Response>::getService;
		using RequestResponse::Client<Request, Response>::getType;
		using RequestResponse::Client<Request, Response>::isValid;
		using RequestResponse::Client<Request, Response>::recv;
		using RequestResponse::Client<Request, Response>::send;
		using RequestResponse::Client<Request, Response>::shutdown;

		int connectDomain(std::string const &domain) {
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
						 (domain + ".").c_str(), C_IN, T_MX, res_res, sizeof(res_res))) <
				0) {
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
			for (auto const &server :
				std::set<std::string>(servers.begin(), servers.end())) {
				try {
					this->connect(Host(server.c_str(), 25));
					connected = true;
					break;
				} catch (...) {
					// Do nothing; just try the next one.
				}
			}
			Response res;
			Socket::recv(res);
			connected = connected && res.code == 220;

			return connected ? 0 : -2;
		}
		int sendEmail(std::string const &ehloParam,
			std::string const &from,
			std::string const &to,
			std::string const &data) {
			Request req;
			Response res;
			req.verb = "EHLO";
			req.parameter = ehloParam;
			Socket::send(req);
			Socket::recv(res);
			if (res.code != 250) {
				return -1;
			}

			req.verb = "MAIL";
			req.parameter = "FROM:<" + from + ">";
			Socket::send(req);
			Socket::recv(res);
			if (res.code != 250) {
				return -2;
			}

			req.verb = "RCPT";
			req.parameter = "TO:<" + to + ">";
			Socket::send(req);
			Socket::recv(res);
			if (res.code != 250) {
				return -3;
			}

			req.verb = "DATA";
			req.parameter = "";
			Socket::send(req);
			Socket::recv(res);
			if (res.code != 354) {
				return -4;
			}

			Socket::send(data);
			Socket::recv(res);
			if (res.code != 250) {
				return -5;
			}

			req.verb = "QUIT";
			req.parameter = "";
			Socket::send(req);
			Socket::recv(res);
			if (res.code != 221) {
				return -6;
			}

			return 0;
		}
	};
}
