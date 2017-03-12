/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef BLOCKAPPS_H_
#define BLOCKAPPS_H_

#include "thread.h"
#include "sharedList.h"

class BlockApps {
private:
	HANDLE hBlockAppsTh;
public:
	SharedList<const char*> dangerousApps;
	HANDLE start();
	void stop();
	HANDLE getHandleThread();
};

#endif /* BLOCKAPPS_H_ */
