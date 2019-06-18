/*
*      Copyright 2016 Riccardo Melioli.
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

    SystemStorage &operator=(const SystemStorage &systemStorage);

    void SaveValueReg(const char *path, const char *key, const char *value);

    std::string LoadValueReg(const char *path, const char *key);

    void SaveValueToFile(std::string name, std::string value, const char *path = NULL);

    std::string LoadValueFromFile(std::string name, const char *path = NULL);

    void RemovePersistence();

    void Persistence();

    bool CheckPersistence();

};

class Session {
private:
    std::map<std::string, std::string> session;
    std::string appName;
    std::string encryptionKey;
    SystemStorage systemStorage;

    void SaveSession(std::string encryptionKey);

    void LoadSession(std::string encryptionKey);

public:
    Session() {}

    Session(std::string encryptionKey, std::string appName);

    Session &operator=(const Session &session);

    void SaveInfo(const char *key, const char *value);

    std::string FindInfo(const char *key);

    void RemoveInfo(const char *key);

    bool CheckSession();

};

#endif /* SESSION_H_ */
