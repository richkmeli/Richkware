/*
 *      Copyright 2016 Riccardo Melioli. All Rights Reserved.
 */

#ifndef SHAREDLIST_H_
#define SHAREDLIST_H_

#include <windows.h>
#include <list>

class Slock {
private:
	CRITICAL_SECTION critical_section;
	BOOL ret;
public:

#ifdef _MSC_VER
	_Acquires_lock_(critical_section)
#endif
	void lock();


#ifdef _MSC_VER
	_Releases_lock_(critical_section)
#endif
	void unlock();


	Slock();
	~Slock();
};

template<class T>
class SharedList {
private:
	std::list<T> list;
	Slock sc;
public:
	void add(T item);
	void remove(T item);
	void clear();
	std::list<T> getCopy();

};

#include "sharedList.templates.h"

#endif /* SHAREDLIST_H_ */
