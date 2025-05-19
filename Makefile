CC = gcc
CXX = 
CFLAGS = -MMD -Wall -I./src
CXXFLAGS = 
CPPFLAGS = 
LDFLAGS = 
LDLIBS = 

BUILD_DIR = build

# Srcs
SRC_DIR = src
SRC_SRCS = $(wildcard $(SRC_DIR)/*.c)
SRC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_SRCS))
SRC_DEPS = $(SRC_OBJS:.o=.d)

# Tests
DEBUG_CFLAGS = -g
TESTS_DIR = tests
TESTS_SRCS = $(wildcard $(TESTS_DIR)/*.c)
TESTS_TARGETS = $(patsubst $(TESTS_DIR)/%.c, $(BUILD_DIR)/%, $(TESTS_SRCS))

# Benches
RELEASE_CFLAGS = -O3
BENCHES_DIR = benches
BENCHES_SRCS = $(wildcard $(BENCHES_DIR)/*.c)
BENCHES_TARGETS = $(patsubst $(BENCHES_DIR)/%.c, $(BUILD_DIR)/%, $(BENCHES_SRCS))

MSYS64_DIR = $(LOCALAPPDATA)/Programs/msys64

## Include paths
BENCHES_INCLUDE_DIRS = \
    -I$(MSYS64_DIR)/mingw64/include \
    -I$(MSYS64_DIR)/usr/include

## Library paths
BENCHES_LIBRARY_DIRS = \
    -L$(MSYS64_DIR)/mingw64/lib \
    -L$(MSYS64_DIR)/usr/lib

## Libs
BENCHES_LIBS = -ljansson

# All
ALL_DEPS = $(SRC_DEPS)
ALL_TARGETS = $(TESTS_TARGETS) $(BENCHES_TARGETS)

# Rules
all: $(ALL_TARGETS)

tests: $(TESTS_TARGETS)

benches: $(BENCHES_TARGETS)

## Build tests
$(BUILD_DIR)/%: $(TESTS_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $< $(SRC_OBJS) -o $@ $(LDFLAGS)

## Build benchmarks
$(BUILD_DIR)/%: $(BENCHES_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(BENCHES_INCLUDE_DIRS) $< $(SRC_OBJS) $(LDFLAGS) $(BENCHES_LIBRARY_DIRS) $(BENCHES_LIBS) -o $@

## Build sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

## Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

## Auto-generated dependency files
-include $(ALL_DEPS)

## Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
