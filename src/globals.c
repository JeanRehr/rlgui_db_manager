/**
 * @file globals.c
 * @brief Globals function implementation and variale values
 */
#include "globals.h"

float window_width = 1600;
float window_height = 800;

void update_window_size(float new_width, float new_height) {
    window_width = new_width;
    window_height = new_height;
}
