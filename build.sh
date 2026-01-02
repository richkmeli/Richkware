#!/bin/bash

# Richkware Build Script for Unix-like systems
# Interactive build configuration

set -e

echo "=== Richkware Build Script ==="
echo

# Function to prompt user for input
prompt_user() {
    local prompt="$1"
    local default="$2"
    local response

    read -p "$prompt [$default]: " response
    echo "${response:-$default}"
}

# Detect current platform
CURRENT_PLATFORM=$(uname -s)
echo "Current platform: $CURRENT_PLATFORM"

# Platform selection
echo
echo "Select target platform:"
echo "1) Current platform ($CURRENT_PLATFORM)"
echo "2) Linux"
echo "3) Windows (Cross-compilation)"
echo "4) macOS"

read -p "Enter choice [1]: " platform_choice
platform_choice=${platform_choice:-1}

case $platform_choice in
    1)
        TARGET_PLATFORM="$CURRENT_PLATFORM"
        ;;
    2)
        TARGET_PLATFORM="Linux"
        ;;
    3)
        TARGET_PLATFORM="Windows"
        ;;
    4)
        TARGET_PLATFORM="macOS"
        ;;
    *)
        echo "Invalid choice. Using current platform."
        TARGET_PLATFORM="$CURRENT_PLATFORM"
        ;;
esac

echo "Target platform: $TARGET_PLATFORM"

# Build type
echo
BUILD_TYPE=$(prompt_user "Build type (Release/Debug)" "Release")

# Build options
echo
echo "Build options:"
ENABLE_TESTS=$(prompt_user "Enable unit tests (ON/OFF)" "ON")
ENABLE_EXAMPLES=$(prompt_user "Enable examples (ON/OFF)" "OFF")
ENABLE_ASAN=$(prompt_user "Enable AddressSanitizer (ON/OFF)" "OFF")
ENABLE_TSAN=$(prompt_user "Enable ThreadSanitizer (ON/OFF)" "OFF")
ENABLE_COVERAGE=$(prompt_user "Enable code coverage (ON/OFF)" "OFF")

# Output directory
BUILD_DIR="build"
if [ "$TARGET_PLATFORM" = "Windows" ]; then
    BUILD_DIR="build_windows"
fi

echo
echo "Build directory: $BUILD_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
echo
echo "Configuring CMake..."

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
CMAKE_ARGS="$CMAKE_ARGS -DRICHKWARE_BUILD_TESTS=$ENABLE_TESTS"
CMAKE_ARGS="$CMAKE_ARGS -DRICHKWARE_BUILD_EXAMPLES=$ENABLE_EXAMPLES"
CMAKE_ARGS="$CMAKE_ARGS -DRICHKWARE_ENABLE_ASAN=$ENABLE_ASAN"
CMAKE_ARGS="$CMAKE_ARGS -DRICHKWARE_ENABLE_TSAN=$ENABLE_TSAN"
CMAKE_ARGS="$CMAKE_ARGS -DRICHKWARE_ENABLE_COVERAGE=$ENABLE_COVERAGE"

# Platform-specific options
if [ "$TARGET_PLATFORM" = "Windows" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake"
    echo "Cross-compiling for Windows using MinGW-w64"
fi

echo "CMake command: cmake .. $CMAKE_ARGS"
cmake .. $CMAKE_ARGS

# Build
echo
echo "Building project..."
make -j$(nproc)

echo
echo "Build completed successfully!"
echo "Executables are in: $BUILD_DIR/bin/"
echo "Libraries are in: $BUILD_DIR/lib/"

# Run tests if enabled
if [ "$ENABLE_TESTS" = "ON" ]; then
    echo
    echo "Running tests..."
    ctest --output-on-failure
fi