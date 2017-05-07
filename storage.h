/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef SESSION_H_
#define SESSION_H_

#include <windows.h>
#include <shellapi.h>
#include <winbase.h>
#include <winreg.h>
#include <winuser.h>

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

#include "crypto.h"


class SystemStorage {
private:
	std::string appName;
public:
	SystemStorage() {}
	SystemStorage(std::string appName);
	SystemStorage& operator=(const SystemStorage& systemStorage);

	void SaveValueReg(const char* path, const char* key, const char* value);
	std::string LoadValueReg(const char* path, const char* key);
	void SaveValueToFile(std::string value, const char* path = NULL);
	std::string LoadValueFromFile(const char* path = NULL);

	void Persistance();
	bool CheckPersistance();

};

class Session{
private:
	std::map<std::string, std::string> session;
	std::string appName;
	const char* encryptionKey;
	SystemStorage systemStorage;

	void SaveSession(const char* encryptionKey);
	void LoadSession(const char* encryptionKey);

public:
	Session() {}
	Session(const char* encryptionKey, std::string appName);
	Session& operator=(const Session& session);

	void SaveInfo(const char* key, const char* value);
	std::string FindInfo(const char* key);
	void RemoveInfo(const char* key);

	bool CheckSession();

};

#endif /* SESSION_H_ */
