#!/bin/bash

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
  mkdir build
fi

# Navigate to build directory
cd build

# Generate build files
cmake ..

# Compile the project
make

# Run the executable
./hyper-grasp
