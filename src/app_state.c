/**
 * @file app_state.c
 * @brief App State function implementations
 */
#include "app_state.h"

const char *app_state_to_string(const enum app_state *const state) {
    switch (*state) {
    case STATE_LOGIN_MENU:
        return "Login Menu";
    case STATE_MAIN_MENU:
        return "Main Menu";
    case STATE_REGISTER_RESIDENT:
        return "Register Resident";
    case STATE_REGISTER_FOOD:
        return "Register Food";
    case STATE_REGISTER_MEDICATION:
        return "Register Medication";
    case STATE_REGISTER_CLOTHES:
        return "Register Clothes";
    case STATE_REGISTER_SUPPLIES:
        return "Register Supplies";
    case STATE_CREATE_USER:
        return "Create User";
    case STATE_SETTINGS:
        return "Settings screen";
    default:
        return "!!!*UNKNOWN MENU*!!!";
    }
}
