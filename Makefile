CXX := g++
CXXFLAGS := -std=c++20
# Conditional compilation flags based on DEBUG variable
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g -O0 -DDK_ENABLE_ASSERTS -Wall -Werror
else
    CXXFLAGS += -O3
endif

BUILD_DIR := ./build
SRC_DIRS := ./Deak/src ./Sandbox/src

# Targets
ENGINE := $(BUILD_DIR)/bin/libDeak.dylib
CLIENT_EXEC := $(BUILD_DIR)/bin/Sandbox

# Automatically find all source files
ENGINE_SRCS := $(shell find ./Deak/src -name '*.cpp')
CLIENT_SRCS := $(shell find ./Sandbox/src -name '*.cpp')
SRCS := $(ENGINE_SRCS) $(CLIENT_SRCS)

# Object files
ENGINE_OBJS := $(ENGINE_SRCS:%=$(BUILD_DIR)/%.o)
CLIENT_OBJS := $(CLIENT_SRCS:%=$(BUILD_DIR)/%.o)
OBJS := $(ENGINE_OBJS) $(CLIENT_OBJS)

# Dependency files
DEPS := $(OBJS:.o=.d)

# Include directories 
INC_DIRS := $(shell find $(SRC_DIRS) -type d) ./Deak/vendor/spdlog/include ./Deak/vendor/GLFW
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Define DK_PLATFORM_MAC and visibility for shared library
CPPFLAGS := -MMD -MP -DDK_PLATFORM_MAC -fvisibility=hidden $(INC_FLAGS)

# Linker flags 
LDFLAGS := -L./Deak/vendor/GLFW/bin -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo

# For linking against the Deak library
LDLIBS := -L$(BUILD_DIR)/bin -lDeak 

.PHONY: all clean

all: $(ENGINE) $(CLIENT_EXEC)

# Rule to build the Deak shared library
$(ENGINE): $(ENGINE_OBJS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) -shared

# Rule to build the Sandbox executable
$(CLIENT_EXEC): $(CLIENT_OBJS) $(ENGINE)
	mkdir -p $(@D)
	$(CXX) $(CLIENT_OBJS) $(CPPFLAGS) -o $@ $(LDLIBS)

# Build step for C++ source files
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)





### OLD MAKEFILES ###

# CXX = g++
# CXXFLAGS = -std=c++20 -Wall -Werror
# PLATFORM = x64
# BUILD_DIR = ./bin

# # ENGINE
# ENGINE_NAME:= Deak
# ENGINE_CXX_FILES = $(wildcard $(ENGINE_NAME)/src/$(ENGINE_NAME)/*.cpp)
# ENGINE_DEFINES:= -fPIC
# ENGINE_INCLUDES:= -I$(ENGINE_NAME)/src
# ENGINE_LINKERS:=
# ENGINE_DIR = $(BUILD_DIR)/$(PLATFORM)/$(ENGINE_NAME)

# # CLIENT
# CLIENT_NAME:= Sandbox
# CLIENT_CXX_FILES = $(wildcard $(CLIENT_NAME)/src/*.cpp)
# CLIENT_DEFINES:=
# CLIENT_INCLUDES:= -I$(CLIENT_NAME)/src -I$(ENGINE_NAME)/src
# CLIENT_LINKERS:= -L$(ENGINE_DIR) -l$(ENGINE_NAME)
# CLIENT_DIR = $(BUILD_DIR)/$(PLATFORM)/$(CLIENT_NAME)

# OBJ_DIR_ENGINE = $(ENGINE_DIR)/obj
# DEP_DIR_ENGINE = $(ENGINE_DIR)/dep

# OBJ_DIR_CLIENT = $(CLIENT_DIR)/obj
# DEP_DIR_CLIENT = $(CLIENT_DIR)/dep



# .PHONY: all client engine clean

# # Engine object and dependency files
# ENGINE_OBJS = $(patsubst $(ENGINE_NAME)/src/$(ENGINE_NAME)/%.cpp,$(OBJ_DIR_ENGINE)/%.o,$(ENGINE_CXX_FILES))
# ENGINE_DEPS = $(patsubst $(ENGINE_NAME)/src/$(ENGINE_NAME)/%.cpp,$(DEP_DIR_ENGINE)/%.d,$(ENGINE_CXX_FILES))

# # Client object and dependency files
# CLIENT_OBJS = $(patsubst $(CLIENT_NAME)/src/%.cpp,$(OBJ_DIR_CLIENT)/%.o,$(CLIENT_CXX_FILES))
# CLIENT_DEPS = $(patsubst $(CLIENT_NAME)/src/%.cpp,$(DEP_DIR_CLIENT)/%.d,$(CLIENT_CXX_FILES))

# # Include dependency files
# -include $(ENGINE_DEPS)
# -include $(CLIENT_DEPS)

# # Engine and client targets
# all: engine client

# engine: $(ENGINE_OBJS)
# 	mkdir -p $(ENGINE_DIR)
# 	$(CXX) $(CXXFLAGS) -shared $(ENGINE_DEFINES) $(ENGINE_INCLUDES) $(ENGINE_LINKERS) $^ -o $(ENGINE_DIR)/lib$(ENGINE_NAME).dylib

# client: $(CLIENT_OBJS)
# 	mkdir -p $(CLIENT_DIR)
# 	$(CXX) $(CXXFLAGS) $(CLIENT_DEFINES) $(CLIENT_INCLUDES) $(CLIENT_LINKERS) $^ -o $(CLIENT_DIR)/$(CLIENT_NAME)

# # Compile source files to object files for engine
# $(OBJ_DIR_ENGINE)/%.o: $(ENGINE_NAME)/src/$(ENGINE_NAME)/%.cpp
# 	mkdir -p $(@D)
# 	mkdir -p $(DEP_DIR_ENGINE)/$(*D)
# 	$(CXX) $(CXXFLAGS) $(ENGINE_DEFINES) $(ENGINE_INCLUDES) -MMD -MP -MF $(DEP_DIR_ENGINE)/$(*F).d -c $< -o $@

# # Compile source files to object files for client
# $(OBJ_DIR_CLIENT)/%.o: $(CLIENT_NAME)/src/%.cpp
# 	mkdir -p $(@D)
# 	mkdir -p $(DEP_DIR_CLIENT)/$(*D)
# 	$(CXX) $(CXXFLAGS) $(CLIENT_DEFINES) $(CLIENT_INCLUDES) -MMD -MP -MF $(DEP_DIR_CLIENT)/$(*F).d -c $< -o $@

# clean:
# 	rm -rf $(BUILD_DIR)






# # Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
# TARGET_EXEC := final_program

# BUILD_DIR := ./build
# SRC_DIRS := ./src

# # Find all the C and C++ files we want to compile
# # Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
# SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

# # Prepends BUILD_DIR and appends .o to every src file
# # As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
# OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# # String substitution (suffix version without %).
# # As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
# DEPS := $(OBJS:.o=.d)

# # Every folder in ./src will need to be passed to GCC so that it can find header files
# INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# # Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
# INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# # The -MMD and -MP flags together generate Makefiles for us!
# # These files will have .d instead of .o as the output.
# CPPFLAGS := $(INC_FLAGS) -MMD -MP

# # The final build step.
# $(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
# 	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# # Build step for C++ source
# $(BUILD_DIR)/%.cpp.o: %.cpp
# 	mkdir -p $(dir $@)
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


# .PHONY: clean
# clean:
# 	rm -r $(BUILD_DIR)

# # Include the .d makefiles. The - at the front suppresses the errors of missing
# # Makefiles. Initially, all the .d files will be missing, and we don't want those
# # errors to show up.
# -include $(DEPS)
