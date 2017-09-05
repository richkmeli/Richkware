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
    std::string encryptionKey;
    SOCKET ListenSocket;
};

struct ClientSocketThreadArgs {
    std::string encryptionKey;
    SOCKET ClientSocket;

};

class Server {
private:
    std::string encryptionKey;
    const char *port;
    HANDLE hThread;
    SOCKET listenSocket;
    ServerThreadArgs sta;
public:
    Server() {}
    Server(std::string encryptionKeyArg);
    Server& operator=(const Server& server);

    void Start(const char* port, bool encrypted = false);
    void Stop();
    HANDLE getHhread();
    const char* getPort();

};

class Network {
private:
    std::string encryptionKey;
public:
    Server server;

    Network() {}
    Network(std::string encryptionKeyArg);
    Network& operator=(const Network& network);

    std::string RawRequest(const char* serverAddress, const char* port, const char* request);
    const char* ResolveAddress(const char* address);
    std::string GetEncryptionKeyFromRMS(const char * serverAddress, const char* port);
    // upload info to Richkware-Manager-Server
    bool UploadInfoToRMS(const char * serverAddress, const char* port);
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
