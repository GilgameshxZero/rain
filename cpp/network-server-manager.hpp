/*
Standard
*/

/*
Implements thread for listening for socket connections and spawning recvThreads.
*/

#pragma once

#include "logstream.hpp"

#include "network-base.hpp"
#include "network-recv-thread.hpp"
#include "network-socket-manager.hpp"
#include "network-utility.hpp"
#include "utility-error.hpp"
#include "utility-logging.hpp"

#include <cstddef>
#include <mutex>

namespace Rain {
	// manages a single server/client connection, from the server-end
	class ServerSocketManager : public SocketManager {
	 public:
		// parameter passed to delegate handlers
		struct DelegateHandlerParam {
			// this object
			ServerSocketManager *ssm;

			// socket of the current connection
			SOCKET *cSocket;

			// message related to current event
			std::string *message;

			// additional parameters set by the creater of the server
			void *callerParam;

			// parameters unique to each ServerSocketManager, which is NULL by default
			// and to be used by the event handlers
			void *delegateParam;
		};

		// ServerSocketManager must be initialized with a ServerManager parent and
		// default event handlers set by the ServerManager's setEventHandlers
		ServerSocketManager(SOCKET *cSocket,
				RecvHandlerParam::EventHandler onConnect,
				RecvHandlerParam::EventHandler onMessage,
				RecvHandlerParam::EventHandler onDisconnect,
				void *funcParam);
		~ServerSocketManager();

		// sends raw message over socket, without queueing or blocking
		void sendRawMessage(std::string request);
		void sendRawMessage(std::string *request);

		// returns current socket immediately
		SOCKET &getSocket();

		// set the delegate event handlers
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
				RecvHandlerParam::EventHandler onMessage,
				RecvHandlerParam::EventHandler onDisconnect,
				void *funcParam);

		// inheirited from SocketManager; sets logging on and off for communications
		// on this socket; pass NULL to disable
		bool setLogging(void *logger);

		// wait until timeout elapses or queued messages are sent
		// 0 for infinite
		void blockForMessageQueue(DWORD msTimeout = 5000);

		// gets the internal handlers, mainly for the use of the ServerManager
		// TODO: fix this
		std::tuple<RecvHandlerParam::EventHandler,
				RecvHandlerParam::EventHandler,
				RecvHandlerParam::EventHandler>
		getInternalHandlers();

	 private:
		SOCKET *socket;
		LogStream *logger;

		// parameter to be passed to delegates
		DelegateHandlerParam ssmdhParam;

		// recvThread parameter associated with the current recvThread
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate,
				onDisconnectDelegate;

		// event handlers for recvThreads spwaned by ServerManager, which will call
		// their delegates set by setEventHandlers of either ServerManager or
		// ServerSocketManager responsible for managing linked list created in
		// ServerManager
		static int onConnect(void *param);
		static int onMessage(void *param);
		static int onDisconnect(void *param);
	};

	// spawns a ServerSocketManager for each new connection
	// A single server should accept a single type of client
	class ServerManager {
	 public:
		typedef void (*NewClientFunc)(ServerSocketManager *);

		// passed to SM's event handlers
		// also used as linked list node for all spawned ServerSocketManagers
		struct RecvThreadParam {
			// used to call the event handler delegates in the SSM
			ServerSocketManager *ssm = NULL;

			// wraps SSM in a linked list node
			RecvThreadParam *prev = NULL, *next = NULL;

			// locked when changing linked list
			std::mutex *llMutex = NULL;

			// storage for the message of this recvThread
			std::string message;

			// handle to the current recvThread for use when waiting on end
			HANDLE hRecvThread = NULL;

			// pointer to the rhParam which has this object as its funcParam, so that
			// the handler can free the recvParam
			Rain::RecvHandlerParam *rhParam = NULL;
		};

		ServerManager();
		~ServerManager();

		// returns current socket immediately
		SOCKET &getSocket();

		// returns current listening port #
		// returns -1 if not yet listening
		DWORD getListeningPort();

		// change socket to listen on different ports, or start socket listen on
		// some ports returns zero if no error pass highPort = 0 to stop server
		// listening
		int setServerListen(DWORD lowPort, DWORD highPort);

		// set the function to call when a new client is connected
		void setNewClientFunc(NewClientFunc newClientCall);

		// set the default delegate event handler for spawned recvThreads
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
				RecvHandlerParam::EventHandler onMessage,
				RecvHandlerParam::EventHandler onDisconnect,
				void *funcParam);
		void *getFuncParam();

		// sets buffer length of spawned recvThreads
		std::size_t setRecvBufLen(std::size_t newLen);

	 protected:
		// default handlers and params which ServerSocketManager should use
		// protected so that 'headed' server manager can change these
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate,
				onDisconnectDelegate;
		void *funcParam;

	 private:
		SOCKET socket;
		DWORD listeningPort;
		DWORD lowPort, highPort;
		NewClientFunc newClientCall;
		std::size_t recvBufLen;
		HANDLE ltEvent;

		void disconnectSocket();

		// creates thread which accepts on a specified listening socket, until the
		// socket is closed from outside the thread accepted connections will spawn
		// a recvThread for each connection with specified parameters any parameters
		// that the onProc functions will need to use will need to be created in
		// onConnect - there will be space to store the parameters to be passed on
		// the functions through the use of a void * in ListenThreadRecvParam when
		// listening socket is closed, thread will close all connection sockets and
		// wait until all spawned recvThreads exit
		static DWORD WINAPI listenThread(LPVOID lpParameter);
	};
}

namespace Rain {
	ServerSocketManager::ServerSocketManager(SOCKET *cSocket,
			RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam) {
		this->socket = cSocket;
		this->logger = NULL;
		this->onConnectDelegate = onConnect;
		this->onMessageDelegate = onMessage;
		this->onDisconnectDelegate = onDisconnect;

		this->ssmdhParam.ssm = this;
		this->ssmdhParam.cSocket = this->socket;
		this->ssmdhParam.message = NULL;
		this->ssmdhParam.callerParam = funcParam;
		this->ssmdhParam.delegateParam = NULL;
	}
	ServerSocketManager::~ServerSocketManager() {
		Rain::shutdownSocketSend(*this->socket);
		closesocket(*this->socket);
		delete this->socket;
	}
	void ServerSocketManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	void ServerSocketManager::sendRawMessage(std::string *request) {
		Rain::sendRawMessage(this->getSocket(), request);

		// if we are logging socket communications, do that here for outgoing
		// communications
		if (this->logger != NULL)
			this->logger->logString(request);
	}
	SOCKET &ServerSocketManager::getSocket() { return *this->socket; }
	void ServerSocketManager::setEventHandlers(
			RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam) {
		this->onConnectDelegate = onConnect;
		this->onMessageDelegate = onMessage;
		this->onDisconnectDelegate = onDisconnect;

		this->ssmdhParam.callerParam = funcParam;
	}
	bool ServerSocketManager::setLogging(void *logger) {
		bool ret = (this->logger != NULL);
		this->logger = reinterpret_cast<LogStream *>(logger);
		return ret;
	}
	void ServerSocketManager::blockForMessageQueue(DWORD msTimeout) {
		// do nothing; server socket managers send when send is called
	}
	std::tuple<RecvHandlerParam::EventHandler,
			RecvHandlerParam::EventHandler,
			RecvHandlerParam::EventHandler>
	ServerSocketManager::getInternalHandlers() {
		return std::make_tuple(
				this->onConnect, this->onMessage, this->onDisconnect);
	}
	int ServerSocketManager::onConnect(void *param) {
		ServerManager::RecvThreadParam &smrtParam =
				*reinterpret_cast<ServerManager::RecvThreadParam *>(param);

		// depending on which delegate handlers are defined, call the right one
		smrtParam.ssm->ssmdhParam.message = &smrtParam.message;
		return smrtParam.ssm->onConnectDelegate == NULL
							 ? 0
							 : smrtParam.ssm->onConnectDelegate(&smrtParam.ssm->ssmdhParam);
	}
	int ServerSocketManager::onMessage(void *param) {
		// call delegate handler
		ServerManager::RecvThreadParam &smrtParam =
				*reinterpret_cast<ServerManager::RecvThreadParam *>(param);

		// if we are logging socket communications, do that here for incoming
		// communications
		if (smrtParam.ssm->logger != NULL)
			smrtParam.ssm->logger->logString(smrtParam.rhParam->message);

		// depending on which delegate handlers are defined, call the right one
		return smrtParam.ssm->onMessageDelegate == NULL
							 ? 0
							 : smrtParam.ssm->onMessageDelegate(&smrtParam.ssm->ssmdhParam);
	}
	int ServerSocketManager::onDisconnect(void *param) {
		ServerManager::RecvThreadParam &smrtParam =
				*reinterpret_cast<ServerManager::RecvThreadParam *>(param);

		// depending on which delegate handlers are defined, call the right one
		int delRtrn =
				smrtParam.ssm->onDisconnectDelegate == NULL
						? 0
						: smrtParam.ssm->onDisconnectDelegate(&smrtParam.ssm->ssmdhParam);

		// modify linked list and remove current ltrfParam
		smrtParam.llMutex->lock();
		smrtParam.prev->next = smrtParam.next;
		smrtParam.next->prev = smrtParam.prev;
		smrtParam.llMutex->unlock();

		// free memory allocated in SM's listenThread
		CloseHandle(smrtParam.hRecvThread);
		delete smrtParam.ssm;
		delete smrtParam.rhParam;
		delete &smrtParam;

		return delRtrn;
	}

	ServerManager::ServerManager() {
		this->funcParam = NULL;
		this->socket = 0;
		this->listeningPort = -1;
		this->lowPort = this->highPort = 0;
		this->onConnectDelegate = this->onMessageDelegate =
				this->onDisconnectDelegate = NULL;
		this->recvBufLen = 65536;

		this->ltEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->newClientCall = NULL;

		Rain::initWinsock22();
	}
	ServerManager::~ServerManager() {
		// stop the listenThread
		this->disconnectSocket();

		// block until the thread exits completely
		WaitForSingleObject(this->ltEvent, 0);
	}
	SOCKET &ServerManager::getSocket() { return this->socket; }
	DWORD ServerManager::getListeningPort() { return this->listeningPort; }
	int ServerManager::setServerListen(DWORD lowPort, DWORD highPort) {
		this->lowPort = lowPort;
		this->highPort = highPort;

		this->disconnectSocket();

		if (highPort == 0) {
			this->listeningPort = -1;
			return 0;
		}

		Rain::createSocket(this->socket);

		// try to bind and listen on each port
		int ret = -1;
		for (DWORD a = this->lowPort; a <= this->highPort; a++) {
			struct addrinfo *addr;
			if (Rain::getServerAddr(&addr, Rain::tToStr(a))) {
				Rain::reportError(WSAGetLastError(),
						"NetworkServerManager error while attempting to getServerAddr");
				continue;
			}
			if (Rain::bindListenSocket(&addr, this->socket)) {
				Rain::reportError(WSAGetLastError(),
						"NetworkServerManager error while attempting to bindListenSocket");
				Rain::freeAddrInfo(&addr);
				continue;
			}

			Rain::freeAddrInfo(&addr);
			this->listeningPort = a;
			ret = 0;
			break;
		}
		if (ret == 0) {	// we successfully listened on a port, so create a thread
										 // to continuously accept clients
			// very important to reset event outside of thread beginning to avoid race
			// conditions
			ResetEvent(this->ltEvent);
			std::thread(listenThread, reinterpret_cast<LPVOID>(this)).detach();
		} else
			ret = WSAGetLastError();
		return ret;
	}
	void ServerManager::setNewClientFunc(NewClientFunc newClientCall) {
		this->newClientCall = newClientCall;
	}
	void ServerManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam) {
		this->onConnectDelegate = onConnect;
		this->onMessageDelegate = onMessage;
		this->onDisconnectDelegate = onDisconnect;
		this->funcParam = funcParam;
	}
	void *ServerManager::getFuncParam() { return this->funcParam; }
	std::size_t ServerManager::setRecvBufLen(std::size_t newLen) {
		std::size_t origValue = this->recvBufLen;
		this->recvBufLen = newLen;
		return origValue;
	}
	DWORD WINAPI ServerManager::listenThread(LPVOID lpParameter) {
		ServerManager &sm = *reinterpret_cast<ServerManager *>(
				lpParameter);	// specific to the listen thread

		// keep track of all WSA2ListenThreadRecvFuncParam spawned so that we can
		// use them to end all recvThreads when thread needs to exit
		RecvThreadParam llHead, llTail;
		llHead.prev = NULL;
		llHead.next = &llTail;
		llTail.prev = &llHead;
		llTail.next = NULL;

		// locked when linked list is being modified
		std::mutex llMutex;

		// listening only ends when it is closed from outside the thread, causing
		// acceptClientSocket to unblock and WSAGetLastError to return WSAINTR
		while (true) {
			// accept a socket
			SOCKET *cSocket = new SOCKET();

			*cSocket = Rain::acceptClientSocket(sm.socket);
			if (*cSocket == INVALID_SOCKET) {
				int error = WSAGetLastError();
				if (error == WSAEINTR) {	// listening closed from outside the thread;
																	// prepare to exit thread
					delete cSocket;
					break;
				} else if (error) {	// unexpected
					Rain::reportError(error,
							"NetworkServerManager error while accepting client sockets");
					return -1;
				}
			}

			// once a client is accepted, spawn a ServerSocketManager and call a
			// function
			ServerSocketManager *ssm =
					new ServerSocketManager(cSocket, sm.onConnectDelegate,
							sm.onMessageDelegate, sm.onDisconnectDelegate, sm.funcParam);
			if (sm.newClientCall != NULL)
				sm.newClientCall(ssm);

			RecvThreadParam *smrtParam = new RecvThreadParam();
			smrtParam->llMutex = &llMutex;
			smrtParam->ssm = ssm;

			// update linked list with new rthParam
			llMutex.lock();
			smrtParam->prev = llTail.prev;
			smrtParam->next = &llTail;
			smrtParam->prev->next = smrtParam;
			llTail.prev = smrtParam;
			llMutex.unlock();

			RecvHandlerParam *rhParam = new RecvHandlerParam();
			rhParam->bufLen = sm.recvBufLen;
			rhParam->funcParam = smrtParam;
			rhParam->message = &smrtParam->message;
			rhParam->socket = cSocket;

			smrtParam->rhParam = rhParam;

			static std::tuple<RecvHandlerParam::EventHandler,
					RecvHandlerParam::EventHandler, RecvHandlerParam::EventHandler>
					internalHandlers;
			internalHandlers = ssm->getInternalHandlers();
			rhParam->onConnect = std::get<0>(internalHandlers);
			rhParam->onMessage = std::get<1>(internalHandlers);
			rhParam->onDisconnect = std::get<2>(internalHandlers);

			// start recvThread for the SSM
			smrtParam->hRecvThread = createRecvThread(rhParam);
		}

		// wait on all spawned and active recvThreads to exit
		llMutex.lock();
		while (llHead.next != &llTail) {
			// close client socket to force blocking WSA2 calls to finish
			// these calls will exit the loop in the RecvThread, prompting
			// ServerManager's onDisconnect to be called, and thus calling CloseHandle
			// on hRecvThread
			SOCKET &socket = llHead.next->ssm->getSocket();
			Rain::shutdownSocketSend(socket);
			closesocket(socket);

			// in case thread gets shutdown while some operations are happening with
			// its handle
			HANDLE curRecvThread = llHead.next->hRecvThread;

			llMutex.unlock();

			// join the recvThread
			// NOTE: the handle may have already been closed at this point
			// TODO fix WaitForSingleObject has undefined behavior if handle is closed
			// during wait, which we do
			CancelSynchronousIo(curRecvThread);
			WaitForSingleObject(curRecvThread, 0);

			llMutex.lock();
		}
		llMutex.unlock();

		// set an event to notify listeners that the thread is exiting
		SetEvent(sm.ltEvent);

		return 0;
	}
	void ServerManager::disconnectSocket() {
		if (this->socket != 0) {
			Rain::shutdownSocketSend(this->socket);
			closesocket(this->socket);
		}
	}
}
