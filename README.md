# Richkware
Malware for Windows, written in C++.

This software is able to:

* Block an application (antivirus, ...);
* Install himself permanently in system and temporary folders; 
* Hide an application;
* Open arbitrary applications;
* Start the server on an arbitrary port, to receive commands from the Internet, thus allowing you to run commands or make changes to application state;
* Send requests to servers;
* Save the application state(encrypted) in the system;
* Hibernate system;
* Move the mouse cursor randomly;
* Require administrator privileges.
	
##COMPILE

###Using g++ compiler (GCC, GNU Compiler Collection)

	g++ -O3 -c -o Richkware.o Richkware.cpp

	g++ -static-libgcc -static-libstdc++ -o Richkware.exe Richkware.o main.o -lws2_32 

###Using Microsoft C++ compiler (Visual Studio)
- C/C++ > Proprocessor > Proprocessor Definitions, add "\_CRT\_SECURE\_NO\_WARNINGS" 
- Linker > Input > Additional Dipendencies, add "Ws2_32.lib"