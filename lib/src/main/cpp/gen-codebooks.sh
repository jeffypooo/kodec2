#!/usr/bin/env bash
NATIVE_PATH=app/src/main/cpp
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
NC='\033[0m'

function verifyclang() {
	echo -e -e "${ORANGE} - Checking Clang installation...${NC}"
	if hash clang 2>/dev/null; then
		clang --version
	else
		while true; do
	    	read -p "Clang is NOT installed. Install it now? [Y/N]" yn
	    	case $yn in
	        [Yy]* ) sudo apt-get install clang; break;;
	        [Nn]* ) exit;;
	        * ) echo -e "Please answer yes or no.";;
	    	esac
		done
	fi
}

function verifycmake() {
	echo -e "${ORANGE} - Checking CMake installation...${NC}"
	if hash cmake 2>/dev/null; then
		cmake /V
	else
		while true; do
	    	read -p "CMake is NOT installed. Install it now? [Y/N]" yn
	    	case $yn in
	        [Yy]* ) sudo apt-get install cmake; break;;
	        [Nn]* ) exit;;
	        * ) echo -e -e "${RED}Please answer yes or no.${NC}";;
	    	esac
		done
	fi
}

echo -e "${ORANGE} - Configuring Codec 2 for Android build...${NC}"

# Update submodules if necessary
if [ ! -f "$NATIVE_PATH/codec2/CMakeLists.txt" ]; then
	echo -e "${ORANGE} - Updating submodules...${NC}"
	git submodule update --init --recursive
fi


# Check Clang/CMake installations
verifyclang
verifycmake

# Point CC/CXX at Clang
export CC=clang
export CXX=clang++

# Create native build dirs if necessary and change to it
CODEC2_BUILD_DIR=$NATIVE_PATH/codec2_native_build
if [ ! -d "$CODEC2_BUILD_DIR" ]; then
	echo -e "${ORANGE} - Creating native build directory...${NC}"
	mkdir $CODEC2_BUILD_DIR
fi
cd $CODEC2_BUILD_DIR

# Generate build tree and make
echo -e "${ORANGE} - Starting build...${NC}"
cmake ../codec2 && make

# Check that codebook generator modules are present
echo -e "${ORANGE} - Verifying CMake modules...${NC}"
if [ ! -f "ImportExecutables.cmake" ]; then 
	echo -e "${RED}CMake modules not found, check CMakeOutput.log for more info${NC}"
	exit 1
else
	echo -e "${GREEN} - Build completed successfully. Please build the project from Android Studio.${NC}"
	exit
fi