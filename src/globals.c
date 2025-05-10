/**
 * @file globals.c
 * @brief Globals function implementation and variale values
 */
#include "globals.h"

int window_width = 1600;
int window_height = 800;

void update_window_size(int new_width, int new_height) {
    window_width = new_width;
    window_height = new_height;
}