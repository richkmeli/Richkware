/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/


#include "network.h"

Server::Server(std::string encryptionKeyArg) {
    encryptionKey = encryptionKeyArg;
    port = "none";
    hThread = NULL;
    listenSocket = INVALID_SOCKET;

    sta.encryptionKey = encryptionKeyArg;
    sta.ListenSocket = INVALID_SOCKET;
}

Server &Server::operator=(const Server &server) {
    encryptionKey = server.encryptionKey;
    port = server.port;
    hThread = server.hThread;
    listenSocket = server.listenSocket;

    sta.encryptionKey = server.sta.encryptionKey;
    sta.ListenSocket = server.sta.ListenSocket;
    return *this;
}

Network &Network::operator=(const Network &network) {
    encryptionKey = network.encryptionKey;
    server = network.server;
    return *this;
}

Network::Network(std::string encryptionKeyArg) {
    encryptionKey = encryptionKeyArg;
    server = Server(encryptionKeyArg);
}

std::string Network::RawRequest(const char *serverAddress, const char *port, const char *request) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    const char *sendbuf = request;
    const int bufferlength = 512;
    char recvbuf[bufferlength];
    int iResult;
    std::string response;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return "Error";
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(serverAddress, port, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return "Error";
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            return "Error";
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return "Error";
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return "Error";
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, bufferlength, 0);
        if (iResult > 0) {
            // answer
            response.append(recvbuf);
        } else if (iResult == 0) {
            // connection closed
        } else {
            // error
        }

    } while (iResult > 0);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return "Error";
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return response;
}


bool Network::UploadInfoToRMS(const char *serverAddress, const char *port) {
    const char *serverPort = server.getPort();
    Crypto crypto(encryptionKey);

    std::string name = getenv("COMPUTERNAME");
    name.append("/");
    name.append(getenv("USERNAME"));

    // encrypt only serverPort because name is used by the server to recognize the device
    std::string serverPortS = crypto.Encrypt(serverPort);
    Device device = Device(name, serverPortS);

    std::string deviceStr = "$" + device.getName() + "," + device.getServerPort() + "$";

    RawRequest(serverAddress, port, ("GET /Richkware-Manager-Server/LoadData?data=" + deviceStr + " HTTP/1.1\r\n"
            "Host: " + serverAddress + "\r\n" +
                                     "Connection: close\r\n" +
                                     "\r\n").c_str());
    return true;
}

std::string Network::GetEncryptionKeyFromRMS(const char *serverAddress, const char *port) {
    Crypto crypto(encryptionKey);
    std::string key = "";

    // create a database entry into the Richkware-Manager-Server, to obtain the encryption key server-side generated
    UploadInfoToRMS(serverAddress, port);

    // Primary key in RMS database.
    std::string name = getenv("COMPUTERNAME");
    name.append("/");
    name.append(getenv("USERNAME"));

    name = crypto.Encrypt(name);
    key = RawRequest(serverAddress, port, ("GET /Richkware-Manager-Server/GetEncryptionKey?id=" + name + " HTTP/1.1\r\n"
            "Host: " + serverAddress + "\r\n" +
                                           "Connection: close\r\n" +
                                           "\r\n").c_str());
    key = key.substr(key.find('$') + 1, (key.find('#') - key.find('$')) - 1);
    key = crypto.Decrypt(key);

    return key;
}

const char *Network::ResolveAddress(const char *address) {
    const char *addressIP = "";
    WSADATA wsaData;
    struct hostent *remoteHost;
    char *host_name;
    struct in_addr addr;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    host_name = (char *) address;
    remoteHost = gethostbyname(host_name);
    if (remoteHost != NULL) {
        if (remoteHost->h_addrtype == AF_INET) {
            int i = 0;
            while (remoteHost->h_addr_list[i] != 0) {
                addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
                addressIP = inet_ntoa(addr);
                break;
            }
        }
    }
    return addressIP;
}


void Server::Start(const char *portArg, bool encrypted) {
    DWORD dwThreadId;
    port = portArg;

    if (encrypted)
        sta.encryptionKey = encryptionKey;
    else
        sta.encryptionKey = "";

    SOCKET listenSocketTmp = INVALID_SOCKET;

    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        //throw 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, portArg, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        //throw 1;
    }

    // Create a SOCKET for connecting to server
    listenSocketTmp = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocketTmp == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        //throw 1;
    }

    // Setup the TCP listening socket
    iResult = bind(listenSocketTmp, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(listenSocketTmp);
        WSACleanup();
        //throw 1;
    }

    freeaddrinfo(result);

    iResult = listen(listenSocketTmp, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        closesocket(listenSocketTmp);
        WSACleanup();
        //throw 1;
    }

    listenSocket = listenSocketTmp;
    sta.ListenSocket = listenSocketTmp;

    hThread = CreateThread(0, 0, &ServerThread,
                           (void *) &sta, 0, &dwThreadId);

}

void Server::Stop() {
    port = "none";

    closesocket(listenSocket);
    WSACleanup();

    DWORD dwExit;
    GetExitCodeThread(hThread, &dwExit);
    TerminateThread(hThread, dwExit);
    //CloseHandle(hThread);
}

HANDLE Server::getHhread() {
    return hThread;
}

const char *Server::getPort() {
    return port;
}

DWORD WINAPI ServerThread(void *arg) {
    std::string encryptionKey = (std::string) ((*((ServerThreadArgs *) arg)).encryptionKey);
    SOCKET ListenSocket = (SOCKET) ((*((ServerThreadArgs *) arg)).ListenSocket);

    //HANDLE hClientThreadArray[1000];
    SOCKET ClientSocket = INVALID_SOCKET;
    while (true) {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        } else {
            ClientSocketThreadArgs* csa = new ClientSocketThreadArgs();
            csa->ClientSocket = ClientSocket;
            csa->encryptionKey = encryptionKey;

            //hClientThreadArray[i] =
            CreateThread(0, 0, &ClientSocketThread, (void *) /*&*/csa, 0, NULL);
        }
    }
    return 0;
}


DWORD WINAPI ClientSocketThread(void *arg) {
    ClientSocketThreadArgs csta = *((ClientSocketThreadArgs *) arg);
    SOCKET ClientSocket = csta.ClientSocket;
    std::string encryptionKey = csta.encryptionKey;

    Crypto crypto(encryptionKey);
    const int bufferlength = 512;
    int iResult;

    std::string command;
    std::string response;
    int iSendResult;
    char recvbuf[bufferlength];
    std::size_t posSubStr;

    // Receive until the peer shuts down the connection
    send(ClientSocket, "\nConnection Established\n", 24, 0);

    do {
        memset(&recvbuf[0], 0, sizeof(recvbuf));
        iResult = recv(ClientSocket, recvbuf, bufferlength, 0);
        command.append(recvbuf);
        //command = recvbuf;

        // string decryption

        if (encryptionKey.compare("") != 0) {
            command.erase(command.find("\r\n"), command.length());
            command = crypto.Decrypt(command);
            command.append("\r\n");
        }

        posSubStr = command.find("\n");
        if (posSubStr != std::string::npos) {
            // erase escape characters
            //command.erase(posSubStr);

            response = CommandsDispatcher(command);

            // string encryption
            if (encryptionKey.compare("") != 0){
                std::string encResponse = (crypto.Encrypt(response) + "\n");
                iSendResult = send(ClientSocket, encResponse.c_str(),encResponse.length(), 0);
            } else{
                iSendResult = send(ClientSocket, response.c_str(),response.length(), 0);
            }


            if (iSendResult == SOCKET_ERROR) {
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        command = "";
        command.clear();

    } while (iResult > 0 && response.compare("***quit***") != 0);

    send(ClientSocket, "\nConnection Stopped\n", 20, 0);

    // shutdown the connection
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

Device::Device(std::string nameArg, std::string serverPortArg) {
    name = nameArg;
    serverPort = serverPortArg;
}

Device &Device::operator=(const Device &device) {
    name = device.name;
    serverPort = device.serverPort;
    return *this;
}

std::string Device::getName() {
    return name;
}

std::string Device::getServerPort() {
    return serverPort;
}

void Device::setName(std::string nameArg) {
    name = nameArg;
}

void Device::setServerPort(std::string serverPortArg) {
    serverPort = serverPortArg;
}
