/**
 * @file dropdownbox.h
 * @brief Selection Dropdown Control
 * 
 * Provides a dropdown selection box with label support and option management.
 * Suitable for:
 * - Category selection
 * - Settings choices
 * - Any discrete option selection
 */

#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include <stdbool.h>

#include <external/raylib/raylib.h>

/**
 * @struct dropdownbox
 * @brief Dropdown selection component
 * 
 * Manages both visual presentation and state for a dropdown selector.
 * Uses RayGUI's GuiDropdownBox internally with added labeling.
 */
struct dropdownbox {
    Rectangle bounds;    ///< Screen position and dimensions { x, y, width, height }
    const char *options; ///< Semicolon-delimited (;) option string (e.g. "Option 1;Option 2;Option 3")
    const char *title;   ///< Descriptive text shown above the dropdown
    int active_option;   ///< Index of currently selected option (0-based)
    bool edit_mode;      ///< Whether the dropdown list is currently expanded
};

/**
 * @brief Initializes a new dropdown box
 * 
 * @param bounds Position and dimensions { x, y, width, height }
 * @param options Pipe-delimited string of available options (required)
 * @param label Descriptive text shown above the dropdown (optional)
 * @return Preconfigured dropdownbox instance with:
 *         - active_option set to 0 (first option)
 *         - edit_mode set to false (collapsed)
 * 
 * @note Options string format: "OPTION1;OPTION2;OPTION3"
 * @warning Both options and label pointers are stored directly (must remain valid)
 */
struct dropdownbox dropdownbox_init(Rectangle bounds, const char *options, const char *title);

/**
 * @brief Draws and manages the dropdown control
 * 
 * Handles:
 * - Label rendering
 * - Dropdown box drawing
 * - Option selection
 * - Expand/collapse toggling
 * 
 * @param ddb Pointer to initialized dropdownbox
 * 
 * @note Call every frame for proper interaction
 * @warning active_option will be updated automatically on user selection
 */
void dropdownbox_draw(struct dropdownbox *ddb);

#endif // DROPDOWNBOX_H
