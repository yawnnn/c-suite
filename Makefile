# Load .env (BASE_LIBS_DIR)
include .env
export

CC = gcc
CXX = 
CFLAGS = -MMD -Wall -I./src
CXXFLAGS = 
CPPFLAGS = 
LDFLAGS = 
LOADLIBES = 
LDLIBS = 

MODE ?= release

ifeq ($(MODE),debug)
	CFLAGS := $(CFLAGS) -g
else ifeq ($(MODE),release)
	CFLAGS := $(CFLAGS) -O3
else
	$(error Unknown MODE "$(MODE)", must be "debug" or "release")
endif

BUILD_DIR = ./build/$(MODE)

# Src
SRC_DIR = ./src
SRC_SRCS = $(wildcard $(SRC_DIR)/*.c)
SRC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_SRCS))
SRC_DEPS = $(SRC_OBJS:.o=.d)

# Tests
TESTS_DIR = ./tests
TESTS_SRCS = $(wildcard $(TESTS_DIR)/*.c)
TESTS_TARGETS = $(patsubst $(TESTS_DIR)/%.c, $(BUILD_DIR)/%, $(TESTS_SRCS))

# Benches
BENCHES_DIR = ./benches
BENCHES_SRCS = $(wildcard $(BENCHES_DIR)/*.c)
BENCHES_TARGETS = $(patsubst $(BENCHES_DIR)/%.c, $(BUILD_DIR)/%, $(BENCHES_SRCS))

BENCHES_INCLUDES = \
    -I$(BASE_LIBS_DIR)/include

BENCHES_LOADLIBES = \
    -L$(BASE_LIBS_DIR)/lib

BENCHES_LDLIBS = -ljansson

# All
ALL_DEPS = $(SRC_DEPS)
ALL_TARGETS = $(TESTS_TARGETS) $(BENCHES_TARGETS)

# Rules
all: $(ALL_TARGETS)

tests: $(TESTS_TARGETS)

benches: $(BENCHES_TARGETS)

## Build tests
$(BUILD_DIR)/%: $(TESTS_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(SRC_OBJS) -o $@ $(LDFLAGS)

## Build benchmarks
$(BUILD_DIR)/%: $(BENCHES_DIR)/%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(BENCHES_INCLUDES) $< $(SRC_OBJS) $(LDFLAGS) $(BENCHES_LOADLIBES) $(BENCHES_LDLIBS) -o $@

## Build sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

## Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

## Run 
run_%: $(BUILD_DIR)/%
	@echo "Running $<"
	$<

## Auto-generated dependency files
-include $(ALL_DEPS)

## Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)