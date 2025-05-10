#include "ui/ui_create_user.h"

static void ui_create_user_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

void ui_create_user_init(struct ui_create_user *ui) {
    ui->base.render = ui_create_user_render;
    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");
}

static void ui_create_user_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db) {
    struct ui_create_user *ui = (struct ui_create_user *)base;

    (void)error;
    (void)user_db;

    if (button_draw_updt(&ui->butn_back)) {
        *state = STATE_MAIN_MENU;
    }
}