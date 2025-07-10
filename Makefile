# Detect OS
UNAME_S := $(shell uname -s)

# Directories
# src directory and subdirectories (find command, works on windows with msys/mingw
# path for them must be higher than the window system32 path)
SRC_DIR = src

# tests directory
TEST_DIR = tests

# output directory for object files
OUT_DIR = out

# The include/ subdirectories are hardcoded into the source files
INCLUDE_DIR = include

# lib directory
LIB_DIR = lib

# Compiler and linker flags per OS
ifeq ($(UNAME_S),Linux)
	CC = clang
	SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
	BASE_LDFLAGS = \
		$(LIB_DIR)/linuxlibraylib.a \
		$(LIB_DIR)/linuxlibcrypto.a \
		$(LIB_DIR)/linuxlibssl.a \
		-lm -lpthread -ldl \
		-lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lXext -lz
else
	CC = gcc
	SAN_FLAGS =
	BASE_LDFLAGS = -L$(LIB_DIR) -lraylib -lopengl32 -lwinmm -lcrypto -lgdi32 -luser32 -lws2_32 -ladvapi32
endif

# Automatically find all source files with find command, maybe not cross-platform?
SRC_FILES = $(shell find $(SRC_DIR) -name "*.c")

# Automatically discover C source files in tests directory
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)

# Transform source file paths to flat object file paths in out/
# Object files for main application (all src files)
MAIN_OUT_FILES = $(addprefix $(OUT_DIR)/,$(notdir $(SRC_FILES:.c=.o)))

# Object files for test suite (test files + src files except main.c)
TEST_OUT_FILES = $(addprefix $(OUT_DIR)/,$(notdir $(TEST_FILES:.c=.o))) $(filter-out $(OUT_DIR)/main.o, $(MAIN_OUT_FILES))

# Compiler and linker flags
# -Weverything
RELEASE_CFLAGS = -O3 -Wall -Wextra -Wpedantic -Werror -std=c11
DEBUG_CFLAGS = -ggdb3 -Wall -Wextra -Wshadow -Wconversion -Wcast-qual -Wformat -Wnull-dereference -Wpedantic -Werror -std=c11 $(SAN_FLAGS)

RELEASE_LDFLAGS = $(BASE_LDFLAGS)
DEBUG_LDFLAGS = $(BASE_LDFLAGS) $(SAN_FLAGS)

INCLUDE_FLAGS = -I$(INCLUDE_DIR)

# Targets
ifeq ($(UNAME_S),Linux)
	MAIN_TARGET = main.out
	TEST_TARGET = tests.out
else
	MAIN_TARGET = main.exe
	TEST_TARGET = tests.exe
endif

# Set default target to debug
.DEFAULT_GOAL := debug

# Build release version
release: CFLAGS = $(RELEASE_CFLAGS)
release: LDFLAGS = $(RELEASE_LDFLAGS)
release: $(MAIN_TARGET) $(TEST_TARGET)

# Build debug version
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
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

# Clean, build release and run app
app:
	$(MAKE) clean
	$(MAKE) release
	./$(MAIN_TARGET)

.PHONY: release debug test run app clean

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

# Disable warnings for style header files 
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@if grep -q '#include.*styles/' $<; then \
		$(CC) $(CFLAGS) -Wno-sign-conversion $(INCLUDE_FLAGS) -c $< -o $@; \
	else \
		$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@; \
	fi

# Repeat for nested directories
$(OUT_DIR)/%.o: $(SRC_DIR)/*/%.c
	@if grep -q '#include.*styles/' $<; then \
		$(CC) $(CFLAGS) -Wno-sign-conversion $(INCLUDE_FLAGS) -c $< -o $@; \
	else \
		$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@; \
	fi

$(OUT_DIR)/%.o: $(SRC_DIR)/*/*/%.c
	@if grep -q '#include.*styles/' $<; then \
		$(CC) $(CFLAGS) -Wno-sign-conversion $(INCLUDE_FLAGS) -c $< -o $@; \
	else \
		$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@; \
	fi

# Special rule for sqlite3.c (disable warnings)
$(OUT_DIR)/sqlite3.o: $(SRC_DIR)/external/sqlite3/sqlite3.c
	$(CC) $(CFLAGS) -w $(INCLUDE_FLAGS) -c $< -o $@

# Add a special rule for raygui.h (disable warnings)
$(OUT_DIR)/raygui.o: $(SRC_DIR)/external/raylib/raygui.c
	$(CC) $(CFLAGS) -w $(INCLUDE_FLAGS) -c $< -o $@

# For test files in tests/
$(OUT_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@
