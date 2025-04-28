/**
 * @file globals.h
 * @brief Global Application State Management
 *
 * Centralized location for application-wide global variables and state management.
 * Provides a single source of truth for shared state across different modules.
 *
 * Current global variables include:
 * - Window dimensions
 *
 * @note All globals should be accessed through their associated update functions
 *       when available to maintain state consistency.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

/* Window Management Globals ***************************************************/

/**
  * @var window_width
  * @brief Current width of the application window in pixels
  * @warning Should only be modified through update_window_size()
  * @ingroup window_management
  */
extern int window_width;

/**
  * @var window_height
  * @brief Current height of the application window in pixels
  * @warning Should only be modified through update_window_size()
  * @ingroup window_management
  */
extern int window_height;

/**
  * @brief Updates the global window dimensions
  *
  * Centralized function for modifying window size state to ensure
  * consistent behavior across all application modules.
  *
  * @param[in] new_width New window width in pixels (must be > 0)
  * @param[in] new_height New window height in pixels (must be > 0)
  * @return void
  * @ingroup window_management
  *
  * @note Emits window resize events to subscribed modules
  * @warning Fails silently if invalid dimensions provided
  */
void update_window_size(int new_width, int new_height);

#endif // GLOBALS_H