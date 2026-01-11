cmake_minimum_required(VERSION 3.20)
project(shelter_management C)

# Export compile_commands.json for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Build Options and Types ----

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# User-configurable build types
set(SHELTER_SUPPORTED_TYPES Debug Release Sanitizer)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# ---- Include directories ----

include_directories(${CMAKE_SOURCE_DIR}/include)

# ---- Gather source files ----

# All .c sources in src/ and subdirectories
file(GLOB_RECURSE SHELTER_SOURCES src/*.c)
# Remove test main and sqlite/raygui special files from normal list
list(FILTER SHELTER_SOURCES EXCLUDE REGEX "src/external/sqlite3/sqlite3.c$")
list(FILTER SHELTER_SOURCES EXCLUDE REGEX "src/external/raylib/raygui.c$")

# Special cases
set(SQLITE3_C ${CMAKE_SOURCE_DIR}/src/external/sqlite3/sqlite3.c)
set(RAYGUI_C  ${CMAKE_SOURCE_DIR}/src/external/raylib/raygui.c)

# Gather test sources
file(GLOB TEST_SOURCES tests/*.c)

# ---- Output directory and file names ----

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

if(WIN32)
    set(TGT_EXT ".exe")
else()
    set(TGT_EXT ".out")
endif()

# ---- Compiler and Linker Flags ----

# Remove all sanitizers from release/debug

set(RELEASE_FLAGS -O3 -Wall -Wextra -Wpedantic -Werror -std=c11 -fno-sanitize=all)
set(DEBUG_FLAGS -g3 -O0 -std=c11 -Wpedantic -Wall -Wextra -Werror -Wshadow -Wconversion -Wcast-qual -Wformat -Wnull-dereference -Wno-switch-default -fno-sanitize=all)
set(SANITIZER_FLAGS -g3 -O0 -std=c11 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wcast-qual -Wformat -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer)

# ---- Define Main Executable ----

add_executable(main
    ${SHELTER_SOURCES}
    ${SQLITE3_C}
    ${RAYGUI_C}
    src/main.c
)

# ---- Set source file properties ----

# Suppress warnings for sqlite3 and raygui and ui_settings.c that includes the styles headers
set_source_files_properties(${SQLITE3_C} PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${RAYGUI_C}  PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${CMAKE_SOURCE_DIR}/src/ui/screens/ui_settings.c  PROPERTIES COMPILE_FLAGS "-w")

# ---- Set compile flags based on build type ----

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    set(BUILD_FLAGS ${DEBUG_FLAGS})
    set(TGT_SUFFIX "-debug")
elseif(CMAKE_BUILD_TYPE MATCHES "[Rr]elease")
    set(BUILD_FLAGS ${RELEASE_FLAGS})
    set(TGT_SUFFIX "-release")
elseif(CMAKE_BUILD_TYPE MATCHES "[Ss]anitizer")
    set(BUILD_FLAGS ${SANITIZER_FLAGS})
    set(TGT_SUFFIX "-sanitizer")
else()
    set(BUILD_FLAGS ${DEBUG_FLAGS})
    set(TGT_SUFFIX "-debug")
endif()

set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_NAME "main${TGT_SUFFIX}"
)
target_compile_options(main PRIVATE ${BUILD_FLAGS})

# ---- Library Linking ----

# Platform-specific extra linker options
if(UNIX AND NOT APPLE) # Linux
    target_link_libraries(main PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/linuxlibraylib.a
        ${CMAKE_SOURCE_DIR}/lib/linuxlibcrypto.a
        ${CMAKE_SOURCE_DIR}/lib/linuxlibssl.a
        m pthread dl
        X11 Xrandr Xinerama Xi Xxf86vm Xcursor Xext z
    )
elseif(WIN32)
    target_link_libraries(main PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/libraylib.a
        winmm
        gdi32
        user32
        ws2_32
        advapi32
        ${CMAKE_SOURCE_DIR}/lib/libcrypto.a
        ${CMAKE_SOURCE_DIR}/lib/libssl.a
        opengl32
    )
endif()

# ---- TEST EXECUTABLE ----

# Use all sources except src/main.c, but add test main instead
set(TEST_EXE_NAME "tests${TGT_SUFFIX}")

# Remove src/main.c from sources for the test binary
set(TEST_SOURCES_LIST ${SHELTER_SOURCES} ${SQLITE3_C} ${RAYGUI_C})
list(FILTER TEST_SOURCES_LIST EXCLUDE REGEX "src/main.c$")

add_executable(tests
    ${TEST_SOURCES_LIST}
    ${TEST_SOURCES} # e.g., tests/tests.c
)

set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_NAME "${TEST_EXE_NAME}"
)

# Propagate compile flags, suppress warnings for libs in tests as well
target_compile_options(tests PRIVATE ${BUILD_FLAGS})
set_source_files_properties(${SQLITE3_C} PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${RAYGUI_C}  PROPERTIES COMPILE_FLAGS "-w")

# Link test binary the same as main
if(UNIX AND NOT APPLE)
    target_link_libraries(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/linuxlibraylib.a
        ${CMAKE_SOURCE_DIR}/lib/linuxlibcrypto.a
        ${CMAKE_SOURCE_DIR}/lib/linuxlibssl.a
        m pthread dl
        X11 Xrandr Xinerama Xi Xxf86vm Xcursor Xext z
    )
elseif(WIN32)
    target_link_libraries(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/libraylib.a
        winmm
        gdi32
        user32
        ws2_32
        advapi32
        ${CMAKE_SOURCE_DIR}/lib/libcrypto.a
        ${CMAKE_SOURCE_DIR}/lib/libssl.a
        opengl32
    )
endif()

# ---- Usage ----
# > mkdir build && cd build
# > cmake -DCMAKE_BUILD_TYPE=Release ..    # or "Debug", or "Sanitizer"
# ON WINDOWS
# > cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
# > cmake --build .
# > ./build/main-release.out (or .exe)
# > ./build/tests-release.out (or .exe)