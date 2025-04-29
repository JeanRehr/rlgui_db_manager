#include <external/raylib/raygui.h>

#include <string.h>

#include "globals.h"
#include "ui/ui_persistent.h"

void ui_persistent_init(struct ui_persistent *ui) {
    ui->logout_butn = button_init((Rectangle) { window_width - 100, window_height - 60, 0, 30 }, "Log Out");

    ui->style_options_bounds = (Rectangle) { window_width - 110, 30, 100, 30 };

    ui->style_options_label =
        (Rectangle) { ui->style_options_bounds.x, ui->style_options_bounds.y - 25, ui->style_options_bounds.width, 20 };

    ui->statusbar_bounds = (Rectangle) { 0, window_height - 20, window_width, 20 };
}

void ui_persistent_draw(struct ui_persistent *ui, struct user *current_user, enum app_state *state, int *active_style) {
    GuiLabel(ui->style_options_label, "Style:");
    GuiComboBox(
        ui->style_options_bounds,
        "Amber;Ashes;Bluish;Candy;Cherry;Cyber;Dark;Enefete;Genesis;Jungle;Lavanda;Light;RLTech;Sunny;Terminal",
        active_style
    );

    if (button_draw_updt(&ui->logout_butn)) {
        memset(current_user, 0, sizeof(struct user));
        *state = STATE_LOGIN_MENU;
    }

    GuiStatusBar(
        ui->statusbar_bounds,
        TextFormat("Logged: %s    Current screen: %s", current_user->username, app_state_to_string(state))
    );
}

void ui_persistent_updt_pos(struct ui_persistent *ui) {
    ui->logout_butn.bounds.x = window_width - 100;
    ui->logout_butn.bounds.y = window_height - 60;

    ui->style_options_bounds.x = window_width - 110;

    ui->style_options_label.x = ui->style_options_bounds.x;

    ui->statusbar_bounds.y = window_height - 20;
    ui->statusbar_bounds.width = window_width;
}