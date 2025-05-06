# Compiler
CC = gcc

# Detect OS
UNAME_S := $(shell uname -s)

# Directories
# src directory and subdirectories
SRC_DIR = src
# Using the find command, the following SRC_* variables are not needed
#SRC_DB_DIR = $(SRC_DIR)/db
#SRC_UI_DIR = $(SRC_DIR)/ui
#SRC_UI_ELEMENTS_DIR = $(SRC_DIR)/ui_elements

# tests directory
TEST_DIR = tests

# output directory
OUT_DIR = out

# The include/ subdirectories are hardcoded into the source files
INCLUDE_DIR = include

# lib directory
LIB_DIR = lib

# Targets
ifeq ($(UNAME_S),Linux)
    MAIN_TARGET = main.out
    TEST_TARGET = tests.out
else
    MAIN_TARGET = main.exe
    TEST_TARGET = tests.exe
endif

# Automatically find all source files with find command, maybe not cross-platform?
SRC_FILES = $(shell find $(SRC_DIR) -name "*.c")

# Automatically discover C source files by manually adding the subdirectory
# Using the above find command, the following is not needed
#SRC_FILES = $(wildcard $(SRC_DB_DIR)/*.c) \
			 $(wildcard $(SRC_UI_DIR)/*.c) \
			 $(wildcard $(SRC_UI_ELEMENTS_DIR)/*.c) \
			 $(wildcard $(SRC_DIR)/*.c)

# Automatically discover C source files in tests directory
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)

# Transform source file paths to flat object file paths in out/
# Object files for main application (all src files)
MAIN_OUT_FILES = $(addprefix $(OUT_DIR)/,$(notdir $(SRC_FILES:.c=.o)))

# Object files for test suite (test files + src files except main.c)
TEST_OUT_FILES = $(addprefix $(OUT_DIR)/,$(notdir $(TEST_FILES:.c=.o))) $(filter-out $(OUT_DIR)/main.o, $(MAIN_OUT_FILES))

# Compiler and linker flags
RELEASE_CFLAGS = -O3 -Wall -Wextra -Werror -pedantic -std=c11 -Wno-missing-braces
DEBUG_CFLAGS = -ggdb3 -Wall -Wextra -Werror -pedantic -std=c11 -Wno-missing-braces
LDFLAGS = -L$(LIB_DIR) -lraylib -lopengl32 -lwinmm -lcrypto -lgdi32 -luser32 -lws2_32 -ladvapi32
INCLUDE_FLAGS = -I$(INCLUDE_DIR)

# Platform-specific flags
ifeq ($(UNAME_S),Linux)
    # Linux static compilation flags
    LDFLAGS = -L$(LIB_DIR) -l:linuxlibraylib.a -l:linuxlibcrypto.a -lz -lm -lpthread -ldl \
              -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lXext \
              -Wl,--no-as-needed -static
else
    # Windows flags
    LDFLAGS = -L$(LIB_DIR) -lraylib -lopengl32 -lwinmm -lcrypto -lgdi32 -luser32 -lws2_32 -ladvapi32
endif

# Set default target to debug
.DEFAULT_GOAL := debug

# Build release version
release: CFLAGS = $(RELEASE_CFLAGS)
release: $(MAIN_TARGET) $(TEST_TARGET)

# Build debug version
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(MAIN_TARGET) $(TEST_TARGET)

# Build debug and run app
run: debug
	./$(MAIN_TARGET)

# Build debug and run test
test: debug
	./$(TEST_TARGET)

# Clean up build artifacts
clean:
	rm -rf $(OUT_DIR)/*.o $(MAIN_TARGET) $(TEST_TARGET)

.PHONY: release debug test run clean

# Build main application
$(MAIN_TARGET): $(MAIN_OUT_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

# Build tests
$(TEST_TARGET): $(TEST_OUT_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

# Rules to compile source files from various locations into flat out directory

# For files directly in src/
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# For files in src/subdir/
$(OUT_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# For files in src/subdir/subsubdir/
$(OUT_DIR)/%.o: $(SRC_DIR)/*/*/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# For test files in tests/
$(OUT_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Makefile Description:
# Variables:\
#	CC: Specifies the compiler (gcc).\
#	RELEASE_CFLAGS: Flags for release builds (includes optimizations and warning settings).\
#	DEBUG_CFLAGS: Flags for debug builds (includes debug symbols for easier debugging with gdb).\
#	SRC_DIR: Directory containing source files (src/).\
#	TEST_DIR: Directory containing test files (tests/).\
#	OUT_DIR: Directory for storing object files (out/).\
#	INCLUDE_DIR: Directory containing header files (include/).\
#	LIB_DIR: Directory containing external libraries (lib/).\
#	SRC_FILES: Finds all .c files in the src directory.\
#	TEST_FILES: Finds all .c files in the tests directory.\
#	OUT_FILES: Object files for both the main application and test executable.\
#	LDFLAGS: Linker flags common to both the main application and test executable.\
#	INCLUDE_FLAGS: Flags specifying include directories.\
# Targets:\
#	release: Builds both the application and test binaries in release mode.\
#	debug: Builds both the application and test binaries in debug mode (default).\
#	clean: Removes generated object files and executables to clean up the directory.\
# Usage:\
#	With this setup in place, the following commands can be used:\
#		make or make debug: Builds both the application and test binaries in debug mode.\
#		make release: Builds both the application and test binaries in release mode.\
#		make clean: Cleans up the directory by removing built executables and object files.\
#		make run: Same as make or make debug, but runs main.exe automatically.\
#		make test: Same as make or make debug, but runs tests.exe automatically.