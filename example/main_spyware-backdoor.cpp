
#include <iostream>

#include "richkware.h"

int main() {
    const char *appName = "Richk";
    const char *defaultEncryptionKey = "richktest"; // pre-shared key with RMS, to enable encryption before receiving a server-side generated key
    const char *serverAddress = "192.168.99.100"; // Richkware-Manager-Server IP address
    const char *port = "8080"; // Richkware-Manager-Server TCP port
    const char *associatedUser = "richk@richk.me"; // account in RMS which is linked to
    int timeoutUploadInfo = 5000;

    // init framework
    Richkware richkware(appName, defaultEncryptionKey, serverAddress, port, associatedUser);

    // start server to receive commands from RMC, on port tcp (6000), and with encryption enabled (true)
    richkware.network.server.Start("6000", true);
    // upload information on RMS, every 5000 seconds
    while (true) {
        richkware.network.UploadInfoToRMS("192.168.99.100", "8080", "richk@richk.me");
        Sleep(timeoutUploadInfo);
    }

    return 0;
}


}


