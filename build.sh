#!/bin/bash
# Build script for Igloo Command Returns
# Sets up SGDK environment and builds the ROM

export SGDK=/c/sgdk
export PATH=/c/sgdk/bin:"/c/Program Files/Java/jre1.8.0_471/bin":$PATH

echo "Building Igloo Command Returns..."
make "$@"
