/**
 * @file listview.h
 * @brief List view values with a scroll
 * 
 * Provides a list view that can also scroll down for long lists.
 * Uses RayGUI's GuiListView internally with enhanced.
 */

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <stdbool.h>

#include <external/raylib/raylib.h>

/**
 * @struct listview
 * @brief List View state handling struct
 *
 */
struct listview {
    Rectangle bounds;    ///< Screen position and dimensions { x, y, width, height }
    const char *title;   ///< Descriptive text shown above the field
    const char *options; ///< Semicolon-delimited (;) option string (e.g. "Option 1;Option 2;Option 3")
    int scrollindex;     ///< Current scroll offset (automatically calculated by RayGui)

    int active_option; ///< Index of currently selected option (0-based)

    int prev_option; ///< Previous selected index (should not be changed, internal tracking to not allow invalid state)
};

/**
 * @brief Initializes a list view struct
 * 
 * @param bounds Position and dimensions { x, y, width, height }
 * @param title Descriptive text shown above the field (required)
 * @param options Pipe-delimited string of available options (required)
 * @return Preconfigured listview instance
 * 
 * - active_option set to 0 (first option)
 * 
 * @note Options string format: "OPTION1;OPTION2;OPTION3"
 *       One option button has a height of ~31
 * 
 * @warning Both options and title pointers are stored directly
 *          (must remain valid for the lifetime of the element)
 */
struct listview listview_init(Rectangle bounds, const char *title, const char *options);

/**
 * @brief Draws and manages the list view
 * Handles:
 * 
 * - Title rendering
 * 
 * - Dropdown box drawing
 * 
 * - Option selection
 * 
 * @param lv Pointer to initialized listview
 * @param unselectable Whether the user can unselect an option
 * 
 * @note Call every frame for proper interaction
 * 
 * @warning active_option will be updated automatically on user selection
 * @warning if unselectable is equals to true, then when a user unselect an
 *          option and active_option will be -1
 */
void listview_draw(struct listview *lv, bool unselectable);

#endif // LISTVIEW_H
