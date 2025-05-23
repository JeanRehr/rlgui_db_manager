/**
 * @file app_state.h
 * @brief Application State Management
 * 
 * Defines the finite state machine states for controlling application flow.
 * Each state represents a distinct UI screen.
 */

#ifndef APP_STATE_H
#define APP_STATE_H

/**
 * @enum app_state
 * @brief Enumerates all possible application states
 * 
 * The state machine controls which screen is currently active.
 */
enum app_state {
    /**
     * @brief Initial login screen state
     * 
     * Authenticates users before granting access to main functionality.
     * Transitions to:
     * - STATE_MAIN_MENU on successful login
     */
    STATE_LOGIN_MENU = 0,

    /**
     * @brief Central hub after authentication
     * 
     * Provides navigation to all major application features.
     * Transitions to:
     * - STATE_REGISTER_RESIDENT
     * - STATE_REGISTER_FOOD
     * - STATE_LOGIN_MENU on logout
     */
    STATE_MAIN_MENU,

    /**
     * @brief Resident registration screen
     * 
     * Handles new resident data entry and validation.
     * Transitions back to:
     * - STATE_MAIN_MENU on going back
     */
    STATE_REGISTER_RESIDENT,

    /**
     * @brief Food inventory registration screen
     * 
     * Manages food batch tracking and storage information.
     * Transitions back to:
     * - STATE_MAIN_MENU on going back
     */
    STATE_REGISTER_FOOD,

    /**
     * @brief Create user screen
     * 
     * Only admins can access this screen.
     * Transitions back to:
     * - STATE_MAIN_MENU on going back
     */
    STATE_CREATE_USER,

    /**
     * @brief Settings screens
     * 
     * Manages the modification of currently logged in user.
     * Also some styling settings.
     * Transitions back to:
     * - STATE_MAIN_MENU on going back
     */
    STATE_SETTINGS,
};

/**
 * @brief Returns a string based on the state screen of the app
 *
 * @param state Pointer to the current state of the app both the pointed-to data is const
 *              (can't modify the enum value through the pointer) and the pointer is const
 *              (can't change what it points to)
 */
const char *app_state_to_string(const enum app_state *const state);

#endif // APP_STATE_H
