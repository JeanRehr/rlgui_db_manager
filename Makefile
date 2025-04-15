# Compiler
CC = gcc

# Directories
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
INCLUDE_DIR = include
LIB_DIR = lib

# Targets
MAIN_TARGET = main.exe
TEST_TARGET = tests.exe

# Automatically discover C source files in src and tests directories
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)

# Object files for main application (all src files)
MAIN_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Object files for test suite (test files + src files except main.c)
TEST_OBJ_FILES = $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_FILES)) \
                 $(filter-out $(OBJ_DIR)/main.o, $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES)))

# Compiler and linker flags
RELEASE_CFLAGS = -O3 -Wall -Wextra -pedantic -std=c99 -Wno-missing-braces
DEBUG_CFLAGS = -g -Wall -Wextra -pedantic -std=c99 -Wno-missing-braces
LDFLAGS = -L$(LIB_DIR) -lraylib -lsqlite3 -lopengl32 -lgdi32 -lwinmm
INCLUDE_FLAGS = -I$(INCLUDE_DIR)

# Set default target to debug
.DEFAULT_GOAL := debug

# Build release version
release: CFLAGS = $(RELEASE_CFLAGS)
release: $(MAIN_TARGET) $(TEST_TARGET)

# Build debug version
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(MAIN_TARGET) $(TEST_TARGET)

# Build main application
$(MAIN_TARGET): $(MAIN_OBJ_FILES)
	$(CC) $(MAIN_OBJ_FILES) -o $(MAIN_TARGET) $(LDFLAGS)

# Build tests
$(TEST_TARGET): $(TEST_OBJ_FILES)
	$(CC) $(TEST_OBJ_FILES) -o $(TEST_TARGET) $(LDFLAGS)

# Compiling source files into objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard $(INCLUDE_DIR)/*.h)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c $(wildcard $(INCLUDE_DIR)/*.h)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ_DIR)/*.o $(MAIN_TARGET) $(TEST_TARGET)

# Makefile Description:
# Variables:\
#	CC: Specifies the compiler (gcc).\
#	RELEASE_CFLAGS: Flags for release builds (includes optimizations and warning settings).\
#	DEBUG_CFLAGS: Flags for debug builds (includes debug symbols for easier debugging with gdb).\
#	SRC_DIR: Directory containing source files (src/).\
#	TEST_DIR: Directory containing test files (tests/).\
#	OBJ_DIR: Directory for storing object files (obj/).\
#	INCLUDE_DIR: Directory containing header files (include/).\
#	LIB_DIR: Directory containing external libraries (lib/).\
#	SRC_FILES: Finds all .c files in the src directory.\
#	TEST_FILES: Finds all .c files in the tests directory.\
#	OBJ_FILES: Object files for both the main application and test executable.\
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