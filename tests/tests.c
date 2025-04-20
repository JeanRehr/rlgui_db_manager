// Including raylib + raygui with #define RAYGUI_IMPLEMENTATION to appease the makefile.
// As makefile dos not links all src/ files expect main.c for the tests directory
// It won't see the RAYGUI_IMPLEMENTATION and will complain
#include <external/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raygui.h>

#include <stdio.h>

#include "utilsfn.h"

int main() {
	int t = 12345;
    char str[10];

    int_to_str(t, str);
    printf("string %s", str);
    return 0;
}