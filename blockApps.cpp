/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "blockApps.h"


HANDLE BlockApps::start() {
	hBlockAppsTh = CreateThread(0, 0, &BlockAppsThread, &dangerousApps, 0, 0);

	//WaitForSingleObject(hBlockAppsTh,INFINITE);

	//CloseHandle(hBlockAppsTh);
	return hBlockAppsTh;
}

void BlockApps::stop() {
	SuspendThread(hBlockAppsTh);
}

HANDLE BlockApps::getHandleThread() {
	return hBlockAppsTh;
}
