#include "app_state.h"

const char* app_state_to_string(const enum app_state const *state) {
    switch (*state) {
    case STATE_LOGIN_MENU:
        return "Login Menu";
    case STATE_MAIN_MENU:
        return "Main Menu";
    case STATE_REGISTER_RESIDENT:
        return "Register Resident";
    case STATE_REGISTER_FOOD:
        return "Register Food";
    default:
        return "!!!UNKNOWN MENU!!!";
    }
}