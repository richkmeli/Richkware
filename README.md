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
	
## **COMPILE**
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -o Richkware.o Richkware.cpp

	g++ -static-libgcc -static-libstdc++ -o Richkware.exe Richkware.o main.o -lws2_32 
