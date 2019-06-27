/*
*      Copyright 2016 Riccardo Melioli.
*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <windows.h>

#include <ctime>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>

std::string CommandsDispatcher(std::string request);

std::string CodeExecution(std::string command);

#endif /* PROTOCOL_H_ */
