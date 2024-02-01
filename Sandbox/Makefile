SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
CC = g++
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
INCLUDE_PATHS = -I../src  # Adjust as needed to point to the headers of the dynamic library
LIBRARY_PATHS = -L../bin/Debug-x64/Deak -L../bin/Release-x64/Deak  # Adjust library paths as needed
CXXFLAGS = -std=c++20 -Wall
PLATFORM = x64
PROJECT_NAME = Sandbox

DEBUG_DIR = $(BIN_DIR)/Debug-$(PLATFORM)/$(PROJECT_NAME)
DEBUG_EXE_NAME = sandbox
DEBUG_FLAGS = -O0 -g -ldeak # Link against the debug version of the dynamic library
DEBUG_EXE_TARGET = $(DEBUG_DIR)/$(DEBUG_EXE_NAME)

RELEASE_DIR = $(BIN_DIR)/Release-$(PLATFORM)/$(PROJECT_NAME)
RELEASE_EXE_NAME = sandbox
RELEASE_FLAGS = -O3 -ldeak  # Link against the release version of the dynamic library
RELEASE_EXE_TARGET = $(RELEASE_DIR)/$(RELEASE_EXE_NAME)

.PHONY: all debug release clean

all: debug release

debug: $(DEBUG_EXE_TARGET)

release: $(RELEASE_EXE_TARGET)

$(DEBUG_EXE_TARGET): $(SRC_FILES)
	mkdir -p $(DEBUG_DIR)
	$(CC) $(CXXFLAGS) $(DEBUG_FLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $^ -o $@ 

$(RELEASE_EXE_TARGET): $(SRC_FILES)
	mkdir -p $(RELEASE_DIR)
	$(CC) $(CXXFLAGS) $(RELEASE_FLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $^ -o $@ 

clean:
	rm -rf $(BIN_DIR)
