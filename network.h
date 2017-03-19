/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef NETWORK_H_
#define NETWORK_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x501
#endif

#define MAX_THREAD 8

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>
#include <ctime>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>

#include "crypto.h"
#include "sharedList.h"

class Server {
private:
	const char* encryptionKey;
	HANDLE hThread;
	//SharedList<HANDLE> hClientThreadArray; 	 BUG1
public:
	Server() {}
	Server(const char* encryptionKeyArg);
	Server& operator=(const Server& server);
	void Start(const char* port, bool encrypted = false);
	void Stop();
	HANDLE getHhread();
	//std::list<HANDLE> getHClientThreadArray(); 	 BUG1
};

class Network {
private:
	const char* encryptionKey;
public:
	Server server;
	Network() {}
	Network(const char* encryptionKeyArg);
	Network& operator=(const Network& network);
	const char* RawRequest(const char* serverAddress, const char* port, const char* request);
	bool UploadInfoToRichkwareManagerServer(const char * serverAddress, const char* port);
};

class Device {
private:
	std::string name;
	std::string serverPort;
public:
	Device(std::string nameArg, std::string serverPort);
	Device& operator=(const Device& device);
	std::string getName();
	std::string getServerPort();
	void setName(std::string nameArg);
	void setServerPort(std::string serverPort);
};

struct ServerThreadArgs {
	const char* port;
	const char* encryptionKey;
	//SharedList<HANDLE> hClientThreadArray; 	 BUG1
	SOCKET ListenSocket;
};

struct ClientSocketThreadArgs {
	SOCKET ClientSocket;
	const char* encryptionKey;
};

DWORD WINAPI ServerThread(void* arg);
DWORD WINAPI ClientSocketThread(void* arg);

#endif /* NETWORK_H_ */
