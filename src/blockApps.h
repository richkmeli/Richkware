/*
*      Copyright 2016 Riccardo Melioli.
*/

#ifndef BLOCKAPPS_H_
#define BLOCKAPPS_H_

#include "sharedResources.h"
#include <chrono>
#include <thread>

class BlockApps {
private:
	HANDLE hBlockAppsTh;
public:
	SharedList<const char*> dangerousApps;

	HANDLE start();
	void stop();
	HANDLE getHandleThread();
};

DWORD WINAPI BlockAppsThread(void* arg);

#endif /* BLOCKAPPS_H_ */
