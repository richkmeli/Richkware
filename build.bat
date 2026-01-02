@echo off
REM Richkware Build Script for Windows
REM Interactive build configuration

echo === Richkware Build Script ===
echo.

REM Check if running on Windows
if "%OS%" neq "Windows_NT" (
    echo This script is for Windows. Use build.sh on Unix-like systems.
    pause
    exit /b 1
)

REM Function to prompt user for input
:PromptUser
setlocal enabledelayedexpansion
set "prompt=%~1"
set "default=%~2"
set "response="

set /p "response=%prompt% [%default%]: "
if "!response!"=="" set "response=%default%"
echo !response!
goto :eof

REM Platform selection
echo.
echo Select target platform:
echo 1) Windows (native)
echo 2) Linux (Cross-compilation)
echo 3) macOS (Cross-compilation)

set /p "platform_choice=Enter choice [1]: "
if "%platform_choice%"=="" set platform_choice=1

if "%platform_choice%"=="1" (
    set TARGET_PLATFORM=Windows
) else if "%platform_choice%"=="2" (
    set TARGET_PLATFORM=Linux
) else if "%platform_choice%"=="3" (
    set TARGET_PLATFORM=macOS
) else (
    echo Invalid choice. Using Windows.
    set TARGET_PLATFORM=Windows
)

echo Target platform: %TARGET_PLATFORM%

REM Build type
echo.
call :PromptUser "Build type (Release/Debug)" "Release"
set BUILD_TYPE=%errorlevel%

REM Build options
echo.
echo Build options:
call :PromptUser "Enable unit tests (ON/OFF)" "ON"
set ENABLE_TESTS=%errorlevel%

call :PromptUser "Enable examples (ON/OFF)" "OFF"
set ENABLE_EXAMPLES=%errorlevel%

call :PromptUser "Enable AddressSanitizer (ON/OFF)" "OFF"
set ENABLE_ASAN=%errorlevel%

call :PromptUser "Enable ThreadSanitizer (ON/OFF)" "OFF"
set ENABLE_TSAN=%errorlevel%

call :PromptUser "Enable code coverage (ON/OFF)" "OFF"
set ENABLE_COVERAGE=%errorlevel%

REM Output directory
set BUILD_DIR=build
if "%TARGET_PLATFORM%"=="Linux" (
    set BUILD_DIR=build_linux
)
if "%TARGET_PLATFORM%"=="macOS" (
    set BUILD_DIR=build_macos
)

echo.
echo Build directory: %BUILD_DIR%

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Configure CMake
echo.
echo Configuring CMake...

set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
set CMAKE_ARGS=%CMAKE_ARGS% -DRICHKWARE_BUILD_TESTS=%ENABLE_TESTS%
set CMAKE_ARGS=%CMAKE_ARGS% -DRICHKWARE_BUILD_EXAMPLES=%ENABLE_EXAMPLES%
set CMAKE_ARGS=%CMAKE_ARGS% -DRICHKWARE_ENABLE_ASAN=%ENABLE_ASAN%
set CMAKE_ARGS=%CMAKE_ARGS% -DRICHKWARE_ENABLE_TSAN=%ENABLE_TSAN%
set CMAKE_ARGS=%CMAKE_ARGS% -DRICHKWARE_ENABLE_COVERAGE=%ENABLE_COVERAGE%

REM Platform-specific options
if "%TARGET_PLATFORM%"=="Linux" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=../cmake/linux-cross.cmake
    echo Cross-compiling for Linux
)
if "%TARGET_PLATFORM%"=="macOS" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=../cmake/macos-cross.cmake
    echo Cross-compiling for macOS
)

echo CMake command: cmake .. %CMAKE_ARGS%
cmake .. %CMAKE_ARGS%

if errorlevel 1 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build
echo.
echo Building project...

REM Try to detect number of cores for parallel build
set CORES=%NUMBER_OF_PROCESSORS%
if "%CORES%"=="" set CORES=4

cmake --build . --config %BUILD_TYPE% --parallel %CORES%

if errorlevel 1 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executables are in: %BUILD_DIR%\bin\
echo Libraries are in: %BUILD_DIR%\lib\

REM Run tests if enabled
if "%ENABLE_TESTS%"=="ON" (
    echo.
    echo Running tests...
    ctest --output-on-failure --build-config %BUILD_TYPE%
)

cd ..
echo.
echo Build script completed. Press any key to exit.
pause >nul