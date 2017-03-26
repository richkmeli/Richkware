/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#include "sharedResources.h"

#ifdef _MSC_VER
	_Acquires_lock_(critical_section)
#endif
void Slock::lock() {
	EnterCriticalSection(&critical_section);
}

#ifdef _MSC_VER
	_Releases_lock_(critical_section)
#endif
void Slock::unlock() {
	LeaveCriticalSection(&critical_section);
}

Slock::Slock() {
	ret = InitializeCriticalSectionAndSpinCount(&critical_section, 0x80000400);
}
Slock::~Slock() {
	DeleteCriticalSection(&critical_section);
}

SharedBool& SharedBool::operator=(const BOOL& rBoolArg) {
	sc.lock();
	rBool = rBoolArg;
	sc.unlock();
	return *this;
}

BOOL SharedBool::getValue() {
	BOOL boolTmp = false;
	sc.lock();
	boolTmp = rBool;
	sc.unlock();
	return boolTmp;
}