#!/bin/bash

# Build and run tests script

echo "Building JsonDiffPatch library and tests..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Build with CMake
cmake ..
cmake --build . --config Release

# Run tests
echo ""
echo "Running tests..."
./run_tests

echo ""
echo "Test run complete!"