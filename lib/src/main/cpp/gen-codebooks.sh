#!/usr/bin/env bash
NATIVE_PATH=./
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
NC='\033[0m'

echo
echo -e "${GREEN}Compiling Codec 2's codebook generator executable for use in cross-compilation builds...${NC}"
echo

# Point CC/CXX at Clang
export CC=clang
export CXX=clang++

# Create native build dirs if necessary and change to it
CODEC2_BUILD_DIR=$NATIVE_PATH/codec2_native_build
if [ ! -d "$CODEC2_BUILD_DIR" ]; then
	echo -e "${ORANGE}Creating native build directory...${NC}"
	mkdir $CODEC2_BUILD_DIR
fi
cd $CODEC2_BUILD_DIR

# Generate build tree and make codebook generator
TARGET=generate_codebook
echo -e "${ORANGE}Running CMake...${NC}"
cmake  --target ${TARGET} -Wno-dev -DUNITTEST=OFF ../codec2
echo

echo -e "${ORANGE}Compiling executable...${NC}"
echo
make -j 4 ${TARGET}
echo

# Check that codebook generator modules are present
echo -e "${ORANGE}Verifying CMake imports...${NC}"
if [ ! -f "ImportExecutables.cmake" ]; then 
	echo -e "${RED}ImportExecutables.cmake not found, check CMakeOutput.log for more info${NC}"
	exit 1
else
	echo -e "${GREEN}Codebook generator binary built successfully.${NC}"
	exit
fi