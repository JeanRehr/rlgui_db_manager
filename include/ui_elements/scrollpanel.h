/**
 * @file scrollpanel.h
 * @brief Scrollable Panel Control
 * 
 * Provides a scrollable container for displaying content larger than the visible area.
 * Uses RayGUI's GuiScrollPanel internally.
 * Suitable for:
 * - Large text displays
 * - Scrolling lists
 * - Any content requiring viewport navigation
 */

#ifndef SCROLLPANEL_H
#define SCROLLPANEL_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

/**
 * @struct scrollpanel
 * @brief Scrollable content container
 * 
 * Manages both visual presentation and scrolling state for a content area.
 * Supports vertical and horizontal scrolling with automatic scrollbar controls.
 */
struct scrollpanel {
    Rectangle panel_bounds;         ///< Outer panel position and dimensions { x, y, width, height }
    const char *title;              ///< Panel title shown in header (optional)
    Rectangle panel_content_bounds; ///< Full content dimensions (may exceed panel_bounds)
    Vector2 scroll;                 ///< Current scroll offset { x, y } (automatically calculated by RayGui)
    Rectangle view;                 ///< Visible content region (automatically calculated by RayGui)
};

/**
 * @brief Initializes a new scrollable panel
 * 
 * @param panel_bounds Outer panel position and dimensions { x, y, width, height }
 * @param title Optional panel title shown in header (may be NULL)
 * @param panel_content_bounds Full content dimensions (may exceed panel_bounds)
 * @return Preconfigured scrollpanel instance
 * 
 * @note Scroll position is automatically initialized to { 0, 0 }
 * @note panel_content_bounds param is dynamically set upon retrieving text based on the Vector2 of text
 * @warning Title pointer is stored directly (must remain valid)
 */
struct scrollpanel scrollpanel_init(Rectangle panel_bounds, const char *title, Rectangle panel_content_bounds);

/**
 * @brief Draws and manages the scrollable panel
 * 
 * Handles:
 * - Panel frame and scrollbar rendering
 * - Scrolling interaction
 * - Content clipping to visible region
 * 
 * @param sp Pointer to initialized scrollpanel
 * @param draw_content Callback function to render panel content
 * @param content_text Text data passed to draw_content callback, safely handles null
 * 
 * @note Call every frame for proper interaction
 * @note @param Rectangle of draw content is calculated inside implementation by the sp fields.
 * @note @param char pointer of draw content is the same as content_text.
 * @warning Content drawing is clipped to visible region
 */
void scrollpanel_draw(struct scrollpanel *sp, void (*draw_content)(Rectangle, char *), char *content_text);

#endif // SCROLLPANEL_H