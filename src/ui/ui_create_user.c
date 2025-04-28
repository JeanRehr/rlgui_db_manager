#include "ui/ui_create_user.h"

void ui_create_user_init(struct ui_create_user *ui) {
    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");
}

void ui_create_user_draw(struct ui_create_user *ui, enum app_state *state, enum error_code *error, database *user_db) {
    if (button_draw_updt(&ui->butn_back)) {
        *state = STATE_MAIN_MENU;
    }
}

void ui_create_user_updt_pos(struct ui_create_user *ui) {
}