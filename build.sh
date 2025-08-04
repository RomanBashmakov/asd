#!/bin/bash

# Build script for STM32 Router project

BUILD_DIR="build"
TOOLCHAIN_PATH="/usr/local/bin"  # Adjust this to your ARM toolchain path

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Files created:"
    ls -la *.elf *.hex *.bin 2>/dev/null || echo "No output files found"
else
    echo "Build failed! Check the error messages above."
    exit 1
fi

cd ..
