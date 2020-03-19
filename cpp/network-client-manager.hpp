/*
Standard
*/

/*
Implements class ClientSocketManager, which maintains a socket connection to an
address & port range, allowing for timeouts and error management along the way.
If a socket connection is terminated, the class will attempt to reconnect until
stopped or successful.
*/

#pragma once

#include "minmax.hpp"
#include "logstream.hpp"
#include "network-base.hpp"
#include "network-recv-thread.hpp"
#include "network-socket-manager.hpp"
#include "network-utility.hpp"
#include "utility-logging.hpp"

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

namespace Rain {
	class ClientSocketManager : public SocketManager {
	 public:
		struct DelegateHandlerParam {
			// the current csm
			ClientSocketManager *csm;

			// message received
			std::string *message;

			// additional parameters set by setHandlers
			void *delegateParam;
		};

		static const int STATUS_DISCONNECTED = -1, STATUS_CONNECTED = 0,
										 STATUS_CONNECTING = 1;

		ClientSocketManager();
		~ClientSocketManager();

		// sends raw message over socket
		// non-blocking by default; if socket is unusable, will delay send until
		// socket is usable again messages are sent without buffering
		void sendRawMessage(std::string request);

		// if blocking, passing by pointer would be faster, avoiding copying of
		// string and sending directly
		void sendRawMessage(std::string *request);

		void clearMessageQueue();

		// wait until timeout elapses or queued messages are sent
		// 0 for infinite
		void blockForMessageQueue(DWORD msTimeout = 5000);

		// set sendRawMessage as blocking or non-blocking
		bool setSendRawMessageBlocking(bool blocking);

		//-1: socket is disconnected; 0: socket is connected; 1: socket is trying to
		// connect but currently unavailable
		int getSocketStatus();

		// blocks until either timeout elapses or socket connects
		// INFINITE for infinite
		void blockForConnect(DWORD msTimeout = 10000);

		// returns value of connected port, or -1 if not connected
		DWORD getConnectedPort();

		// returns domain name or IP, whichever was set in setClientTarget, or "" if
		// not connected
		std::string getTargetIP();

		// returns current socket immediately
		SOCKET &getSocket();

		// if called with non-empty ipAddress, setClientTarget will tell class to
		// begin trying to connect to the target an empty ipAddress terminates any
		// existing connection and will not reconnect if called, will terminate
		// current connection
		void setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort);

		// if disconnected, start trying to connect again
		void retryConnect();

		// whether to retry connection when disconnect; returns previous value
		bool setRetryOnDisconnect(bool value);

		// set event handlers in addition to those of class
		// pass NULL to any parameter to remove the custom handler
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
				RecvHandlerParam::EventHandler onMessage,
				RecvHandlerParam::EventHandler onDisconnect,
				void *funcParam);

		// set reconnect attempt time max from default 3000
		// will attempt to reconnect more often at the beginning, then slow down
		// exponentially
		DWORD setReconnectAttemptTime(DWORD msMaxInterval);

		// set send message attempt time max
		DWORD setSendAttemptTime(DWORD msMaxInterval);

		// sets buffer length of recv thread
		std::size_t setRecvBufLen(std::size_t newLen);

		// inheirited from SocketManager; sets logging on and off for communications
		// on this socket; pass NULL to disable
		bool setLogging(void *logger);

	 protected:
		// have the message handlers be protected so that derived classes can access
		// them
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate,
				onDisconnectDelegate;

		// parameter passed to delegate handlers
		DelegateHandlerParam csmdhParam;

	 private:
		SOCKET socket;
		std::queue<std::string> messageQueue;
		bool blockSendRawMessage;
		DWORD connectedPort;
		int socketStatus;
		std::string ipAddress;
		DWORD lowPort, highPort;
		DWORD msReconnectWaitMax, msSendWaitMax;
		std::size_t recvBufLen;
		LogStream *logger;

		// set to true when destructor called; terminate send thread here
		bool destructing;

		// current wait between connect attempts
		DWORD msReconnectWait;

		// current wait between send message attempts
		DWORD msSendMessageWait;

		// addresses of all the ports
		std::vector<struct addrinfo *> portAddrs;

		// will be triggerred while socket is connected
		HANDLE connectEvent;

		// triggerred when message queue is empty
		HANDLE messageDoneEvent;

		// triggered when message queue not empty again
		HANDLE messageToSendEvent;

		// recvThread parameter associated with the current recvThread
		Rain::RecvHandlerParam rParam;

		// handle to the connect thread and send message thread, and the recv thread
		// close when threads end by the thread
		HANDLE hConnectThread, hSendThread, hRecvThread;

		// event which will be set when there have been as many calls to
		// onDisconnect as there have been to onConnect
		HANDLE recvExitComplete;

		// lock this when modifying message queue
		std::mutex queueMutex;

		// if set to true, CSM will attempt to reconnect on disconnect ONCE
		bool retryOnDisconnect;

		// disconnects socket immediately, regardless of state
		// sets state as -1
		void disconnectSocket();

		// frees all the memory allocated for addresses
		void freePortAddrs();

		// thread function to attempt reconnects with time intervals
		static DWORD WINAPI attemptConnectThread(LPVOID param);

		// thread function to attempt send messages on an interval
		static DWORD WINAPI attemptSendMessageThread(LPVOID param);

		// event handlers for internal recvThread, before passing to delegates
		static int onConnect(void *param);
		static int onMessage(void *param);
		static int onDisconnect(void *param);
	};
}

namespace Rain {
	ClientSocketManager::ClientSocketManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->connectedPort = -1;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnectDelegate = this->onMessageDelegate =
				this->onDisconnectDelegate = NULL;
		this->msReconnectWaitMax = this->msSendWaitMax = 3000;
		this->recvBufLen = 65536;
		this->logger = NULL;

		this->retryOnDisconnect = false;

		this->hConnectThread = this->hSendThread = NULL;

		this->csmdhParam.csm = this;
		this->csmdhParam.delegateParam = NULL;
		this->csmdhParam.message = new std::string();

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		this->messageToSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->recvExitComplete = CreateEvent(NULL, TRUE, TRUE, NULL);

		this->destructing = false;

		int error = initWinsock22();
		if (error) {
			reportError(error,
					"ClientSocketManager: initWinsock22 failed, no resolution "
					"implemented");
		}

		// create the send thread once for every manager
		this->hSendThread =
				simpleCreateThread(ClientSocketManager::attemptSendMessageThread, this);
	}
	ClientSocketManager::~ClientSocketManager() {
		// don't try to reconnect
		this->retryOnDisconnect = false;

		// shutdown send threads, if any
		this->destructing = true;
		this->clearMessageQueue();
		ResetEvent(this->messageDoneEvent);
		SetEvent(this->messageToSendEvent);
		WaitForSingleObject(this->messageDoneEvent, INFINITE);

		// shutsdown connect threads, if any
		this->disconnectSocket();

		// wait for recvThread to exit
		WaitForSingleObject(this->hRecvThread, INFINITE);
		CloseHandle(this->hRecvThread);

		// critical: free addrs after shutting down connect threads, or there will
		// be multithreading problems
		this->freePortAddrs();

		if (this->csmdhParam.message != NULL)
			delete this->csmdhParam.message;
		this->csmdhParam.message = NULL;
	}
	void ClientSocketManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	void ClientSocketManager::sendRawMessage(std::string *request) {
		// uses copy constructor
		// reset event outside of thread start so multiple calls to sendRawMessage
		// won't create race conditions
		this->queueMutex.lock();
		if (this->messageQueue.size() == 0) {
			ResetEvent(this->messageDoneEvent);
			SetEvent(this->messageToSendEvent);
		}
		this->messageQueue.push(*request);
		this->queueMutex.unlock();

		// if we are logging socket communications, do that here for outgoing
		// communications
		if (this->logger != NULL)
			this->logger->logString(request);
		if (this->blockSendRawMessage)
			this->blockForMessageQueue(0);
	}
	void ClientSocketManager::clearMessageQueue() {
		this->queueMutex.lock();
		this->messageQueue = std::queue<std::string>();
		this->queueMutex.unlock();
	}
	void ClientSocketManager::blockForMessageQueue(DWORD msTimeout) {
		WaitForSingleObject(this->messageDoneEvent, msTimeout);
	}
	bool ClientSocketManager::setSendRawMessageBlocking(bool blocking) {
		bool origValue = this->blockSendRawMessage;
		this->blockSendRawMessage = blocking;
		return origValue;
	}
	int ClientSocketManager::getSocketStatus() { return this->socketStatus; }
	void ClientSocketManager::blockForConnect(DWORD msTimeout) {
		WaitForSingleObject(this->connectEvent, msTimeout);
	}
	DWORD ClientSocketManager::getConnectedPort() { return this->connectedPort; }
	std::string ClientSocketManager::getTargetIP() { return this->ipAddress; }
	SOCKET &ClientSocketManager::getSocket() { return this->socket; }
	void ClientSocketManager::setClientTarget(std::string ipAddress,
			DWORD lowPort,
			DWORD highPort) {
		this->disconnectSocket();
		this->ipAddress = ipAddress;

		if (this->ipAddress.length() == 0) {
			this->lowPort = this->highPort = 0;
		} else {
			this->lowPort = lowPort;
			this->highPort = highPort;

			this->socketStatus = this->STATUS_CONNECTING;

			// create thread to attempt connect
			// thread exits when connect success
			ResetEvent(this->connectEvent);
			this->hConnectThread =
					simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	void ClientSocketManager::retryConnect() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			// create thread to attempt connect
			// thread exits when connect success
			ResetEvent(this->connectEvent);
			this->hConnectThread =
					simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	bool ClientSocketManager::setRetryOnDisconnect(bool value) {
		bool orig = this->retryOnDisconnect;
		this->retryOnDisconnect = value;
		return orig;
	}
	void ClientSocketManager::setEventHandlers(
			RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam) {
		this->onConnectDelegate = onConnect;
		this->onMessageDelegate = onMessage;
		this->onDisconnectDelegate = onDisconnect;
		this->csmdhParam.delegateParam = funcParam;
	}
	DWORD ClientSocketManager::setReconnectAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msReconnectWaitMax;
		this->msReconnectWaitMax = msMaxInterval;
		return origValue;
	}
	DWORD ClientSocketManager::setSendAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msSendWaitMax;
		this->msSendWaitMax = msMaxInterval;
		return origValue;
	}
	std::size_t ClientSocketManager::setRecvBufLen(std::size_t newLen) {
		std::size_t origValue = this->recvBufLen;
		this->recvBufLen = newLen;
		return origValue;
	}
	bool ClientSocketManager::setLogging(void *logger) {
		bool ret = (this->logger != NULL);
		this->logger = reinterpret_cast<LogStream *>(logger);
		return ret;
	}
	void ClientSocketManager::disconnectSocket() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			return;
		} else if (this->socketStatus == this->STATUS_CONNECTED ||
							 this->socketStatus ==
									 this->STATUS_CONNECTING) {	// connected, so shutdown
																							 // immediately or terminate thread
																							 // on next check
			shutdownSocketSend(this->socket);
			closesocket(this->socket);

			// this will exit the connect thread
			this->socketStatus =
					this->STATUS_DISCONNECTED;	// manually disconnect, so that we won't
																			// try to disconnect
			WaitForSingleObject(this->connectEvent, INFINITE);

			this->connectedPort = -1;

			// wait for onDisconnect if socket was connected before
			WaitForSingleObject(this->recvExitComplete, INFINITE);
		}
	}
	void ClientSocketManager::freePortAddrs() {
		for (std::size_t a = 0; a < this->portAddrs.size(); a++) {
			freeAddrInfo(&portAddrs[a]);
		}
		this->portAddrs.clear();
	}
	DWORD WINAPI ClientSocketManager::attemptConnectThread(LPVOID param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		// resetting the connect event should be placed outside this thread, before
		// it is created, to make sure that any calls to the thread are waited upon
		// as intended

		csm.msReconnectWait = 1;
		csm.freePortAddrs();
		csm.portAddrs.resize(static_cast<size_t>(csm.highPort) -
														 static_cast<size_t>(csm.lowPort) + 1,
				NULL);

		if (createSocket(csm.socket)) {
			reportError(WSAGetLastError(),
					"ClientSocketManager: createSocket failed, aborting...");
			SetEvent(csm.connectEvent);
			return -1;
		}

		while (csm.socketStatus == csm.STATUS_CONNECTING) {
			Rain::sleep(csm.msReconnectWait);
			if (csm.msReconnectWait < csm.msReconnectWaitMax)
				csm.msReconnectWait =
						min(csm.msReconnectWait * 2, csm.msReconnectWaitMax);

			// try to reconnect; assume ipAddress and port ranges are valid
			// if address has not yet been retreived, try to get it here
			for (DWORD a = csm.lowPort; a <= csm.highPort; a++) {
				if (csm.socketStatus !=
						csm.STATUS_CONNECTING)	// make sure status hasn't changed
																		// externally
					break;

				if (csm.portAddrs[a - csm.lowPort] ==
						NULL) {	// address not yet found, get it now
					if (getTargetAddr(&csm.portAddrs[a - csm.lowPort], csm.ipAddress,
									Rain::tToStr(a))) {
						reportError(WSAGetLastError(),
								"ClientSocketManager: getTargetAddr failed, retrying...");
						continue;
					}
				}

				// any socket that is connected will pass through this step
				if (!connectTarget(csm.socket, &csm.portAddrs[a - csm.lowPort])) {
					csm.connectedPort = a;
					csm.socketStatus = csm.STATUS_CONNECTED;

					// attach the socket to a recvThread
					csm.rParam.bufLen = csm.recvBufLen;
					csm.rParam.funcParam = static_cast<void *>(&csm);
					csm.rParam.message = csm.csmdhParam.message;
					csm.rParam.onMessage = csm.onMessage;
					csm.rParam.onConnect = csm.onConnect;
					csm.rParam.onDisconnect = csm.onDisconnect;
					csm.rParam.socket = &csm.socket;

					// save the recvThread; we need to wait on it later
					csm.hRecvThread = createRecvThread(&csm.rParam);

					break;
				} else {
					reportError(WSAGetLastError(),
							"ClientSocketManager: connectTarget failed, retrying...");
					std::cerr << csm.socket << Rain::CRLF;
					continue;
				}
			}
		}

		// this event signals that the thread has exited; check status to see if
		// successful
		SetEvent(csm.connectEvent);
		return 0;
	}
	DWORD WINAPI ClientSocketManager::attemptSendMessageThread(LPVOID param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		while (!csm.destructing) {
			// wait until we know there are messages in queue
			WaitForSingleObject(csm.messageToSendEvent, INFINITE);

			csm.msSendMessageWait = 1;

			// attempt to send messages until csm.messageQueue is empty, at which
			// point set the messageDoneEvent
			csm.queueMutex.lock();
			while (!csm.messageQueue.empty()) {
				csm.blockForConnect(csm.msSendMessageWait);

				// deal with differently depending on whether we connected on that block
				// or not
				if (csm.socketStatus != csm.STATUS_CONNECTED &&
						csm.msSendMessageWait < csm.msSendWaitMax)
					csm.msSendMessageWait =
							min(csm.msSendMessageWait * 2, csm.msSendWaitMax);
				else if (csm.socketStatus == csm.STATUS_CONNECTED) {
					Rain::sendRawMessage(csm.socket, &csm.messageQueue.front());
					csm.messageQueue.pop();
				}
				csm.queueMutex.unlock();
				// unlock temporarily to allow other functions to maybe continue
				csm.queueMutex.lock();
			}
			ResetEvent(csm.messageToSendEvent);
			csm.queueMutex.unlock();
			SetEvent(csm.messageDoneEvent);
		}

		return 0;
	}
	int ClientSocketManager::onConnect(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		// reset an event for listeners of the end of onDisconnect
		ResetEvent(csm.recvExitComplete);

		return csm.onConnectDelegate == NULL
							 ? 0
							 : csm.onConnectDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onMessage(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		// if we are logging socket communications, do that here for incoming
		// messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessageDelegate == NULL
							 ? 0
							 : csm.onMessageDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onDisconnect(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		// if set, we want to try to reconnect
		if (csm.retryOnDisconnect) {
			csm.socketStatus = csm.STATUS_CONNECTING;

			// create thread to attempt connect
			// thread exits when connect success
			ResetEvent(csm.connectEvent);
			csm.hConnectThread =
					simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);
		} else {
			csm.socketStatus = csm.STATUS_DISCONNECTED;

			// set an event to listeners, that this function has been called
			SetEvent(csm.recvExitComplete);
		}

		// call delegate if it exists regardless of disconnect attempt; this may
		// activate CSM destructor; but it should block on recvThread
		return csm.onDisconnectDelegate == NULL
							 ? 0
							 : csm.onDisconnectDelegate(&csm.csmdhParam);
	}
}
