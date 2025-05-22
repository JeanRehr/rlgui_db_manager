/**
 * @file raygui.c
 * @brief Only a translation unit for the purpose of compiling the implementation of raygui
 *
 * With this there is no need to recompile for main and tests
 * (also supress warnings without changing the source)
 */
#include <external/raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raylib/raygui.h>