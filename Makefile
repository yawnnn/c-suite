CC = gcc
CXX = 
CFLAGS = -MMD -Wall -I./src
CXXFLAGS = 
CPPFLAGS = 
LDFLAGS = 
LDLIBS = 

# Directories
MSYS64_DIR = $(LOCALAPPDATA)/msys64
SRC_DIR = src
TESTS_DIR = tests
BENCHES_DIR = benches
BUILD_DIR = build

DEBUG_CFLAGS = -g
RELEASE_CFLAGS = -O3

# Include Directories
INCLUDE_DIRS = \
    -I$(MSYS64_DIR)/mingw64/include \
    -I$(MSYS64_DIR)/usr/include

# Library Directories
LIBRARY_DIRS = \
    -L$(MSYS64_DIR)/mingw64/lib \
    -L$(MSYS64_DIR)/usr/lib

# Files
SRC_SRCS =  $(wildcard $(SRC_DIR)/*.c)
SRC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_SRCS))
SRC_DEPS = $(SRC_OBJS:.o=.d)

# Tests
TESTS_SRCS = $(wildcard $(TESTS_DIR)/*.c)
TESTS_TARGETS = $(patsubst $(TESTS_DIR)/%.c, $(BUILD_DIR)/%, $(TESTS_SRCS))

# Benchmarks
BENCHES_SRCS = $(wildcard $(BENCHES_DIR)/*.c)
BENCHES_TARGETS = $(patsubst $(BENCHES_DIR)/%.c, $(BUILD_DIR)/%, $(BENCHES_SRCS))

# Per-target libraries for benchmarks
LIBS_allocators = -ljansson

# All dependencies and targets
ALL_DEPS = $(SRC_DEPS)
ALL_TARGETS = $(TESTS_TARGETS) $(BENCHES_TARGETS)

# Rules
all: $(ALL_TARGETS)

examples: $(TESTS_TARGETS)

benches: $(BENCHES_TARGETS)

# Rule to build example executables
$(BUILD_DIR)/%: $(TESTS_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $< $(SRC_OBJS) -o $@ $(LDFLAGS)

# Rule to build benchmark executables
$(BUILD_DIR)/%: $(BENCHES_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $(INCLUDE_DIRS) $< $(SRC_OBJS) $(LDFLAGS) $(LIBRARY_DIRS) $(LIBS_$(*F)) -o $@

# Rule to build source .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Include dependency files
-include $(ALL_DEPS)

# Clean rule
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
