/**
 * @file app_state.c
 * @brief App State function implementations
 */
#include "global/app_state.h"

const char *app_state_to_string(const enum app_state *const state) {
    switch (*state) {
    case STATE_LOGIN_MENU:
        return "Login Menu";
    case STATE_MAIN_MENU:
        return "Main Menu";
    case STATE_MANAGE_RESIDENT:
        return "Manage Resident";
    case STATE_MANAGE_FOOD:
        return "Manage Food";
    case STATE_MANAGE_MEDICATION:
        return "Manage Medication";
    case STATE_MANAGE_CLOTHES:
        return "Manage Clothes";
    case STATE_MANAGE_SUPPLIES:
        return "Manage Supplies";
    case STATE_CREATE_USER:
        return "Create User";
    case STATE_SETTINGS:
        return "Settings screen";
    default:
        return "!!!*UNKNOWN MENU*!!!";
    }
}
