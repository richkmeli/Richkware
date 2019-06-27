/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "src/richkware.h"
#include <assert.h>

int main() {
    const char *appName = "Richk";
    const char *defaultEncryptionKey = "richktest"; // pre-shared key with RMS, to enable encryption before receiving a server-side generated key
    const char *serverAddress = "127.0.0.1"; // Richkware-Manager-Server IP address
    const char *port = "8080"; // Richkware-Manager-Server TCP port
    const char *associatedUser = "richk@richk.me"; // account in RMS which is linked to
    int timeoutUploadInfo = 5000;


    // with RichkwareManagerServer
    Richkware richkware(appName, defaultEncryptionKey, serverAddress, port, associatedUser);
    // without RichkwareManagerServer
    //Richkware richkware(appName, defaultEncryptionKey);

//-----------


    // PUT YOUR CODE HERE
    richkware.network.server.Start("6000", true);
    // upload information on RMS, every 5000 seconds

    //while (true) {
    for (int i = 0; i < 2; i++) {
        richkware.network.UploadInfoToRMS();
        Sleep(timeoutUploadInfo);
    }

    system("pause");
    return 0;

}

