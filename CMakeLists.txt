cmake_minimum_required(VERSION 3.7)
project(Richkware)

if (UNIX)
    set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
endif (UNIX)

set(CMAKE_CXX_STANDARD 98)

set(CMAKE_STATIC_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#link_libraries(kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32 wsock32 ws2_32 rpcrt4 wininet)
link_libraries(ws2_32)

set(SOURCE_FILES
        blockApps.cpp
        blockApps.h
        crypto.cpp
        crypto.h
        main.cpp
        network.cpp
        network.h
        protocol.cpp
        protocol.h
        richkware.cpp
        richkware.h
        sharedResources.cpp
        sharedResources.h
        sharedResources.templates.h
        storage.cpp
        storage.h)

add_executable(Richkware ${SOURCE_FILES})