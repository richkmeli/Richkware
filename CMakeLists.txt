cmake_minimum_required(VERSION 3.7)
project(Richkware)

# Cross-compile for Windows from Linux using MinGW
if (UNIX)
    set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
endif (UNIX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_STATIC_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# Windows libraries
#link_libraries(kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32 wsock32 ws2_32 rpcrt4 wininet)
link_libraries(ws2_32)

# NOTE: OpenSSL for MinGW cross-compilation
# The AES implementation (aes_openssl.cpp) requires OpenSSL headers/libraries
# For cross-compiling to Windows, you need mingw-w64 OpenSSL packages
# On Ubuntu/Debian: sudo apt-get install mingw-w64-x86-64-dev libssl-mingw-w64-dev (if available)
# Alternatively, build OpenSSL for MinGW manually or use a pre-built MinGW OpenSSL package

include_directories(include)

set(PROJECT_SOURCE_DIR src)

# Gather all .cpp files except the old custom aes.cpp (we use aes_openssl.cpp)
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h main.cpp)
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/aes.cpp")

add_executable(Richkware ${SOURCE_FILES})