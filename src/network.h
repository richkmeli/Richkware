/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef NETWORK_H_
#define NETWORK_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x501
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

#include "crypto.h"
#include "protocol.h"

struct ServerThreadArgs {
    const char* encryptionKey;
    SOCKET ListenSocket;
};

struct ClientSocketThreadArgs {
    SOCKET ClientSocket;
    const char* encryptionKey;
};

class Server {
private:
    const char* encryptionKey;
    const char* port;
    HANDLE hThread;
    SOCKET listenSocket;
    ServerThreadArgs sta;
public:
    Server() {}
    Server(const char* encryptionKeyArg);
    Server& operator=(const Server& server);

    void Start(const char* port, bool encrypted = false);
    void Stop();
    HANDLE getHhread();
    const char* getPort();

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
    const char* ResolveAddress(const char* address);
    bool UploadInfoToRichkwareManagerServer(const char * serverAddress, const char* port);
};

class Device {
private:
    std::string name;
    std::string serverPort;
public:
    Device() {}
    Device(std::string nameArg, std::string serverPort);
    Device& operator=(const Device& device);

    std::string getName();
    std::string getServerPort();
    void setName(std::string nameArg);
    void setServerPort(std::string serverPort);
};

DWORD WINAPI ServerThread(void* arg);
DWORD WINAPI ClientSocketThread(void* arg);

#endif /* NETWORK_H_ */
