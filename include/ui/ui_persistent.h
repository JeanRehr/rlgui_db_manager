#ifndef UI_PERSISTENT_H
#define UI_PERSISTENT_H

#include "app_state.h"
#include "user.h"
#include "ui_elements/button.h"

struct ui_persistent {
    struct button logout_butn;

    Rectangle style_options_bounds;
    Rectangle style_options_label;
    Rectangle statusbar_bounds;
};

void ui_persistent_init(struct ui_persistent *ui);

void ui_persistent_draw(struct ui_persistent *ui, struct user *current_user, enum app_state *state, int *active_style);

void ui_persistent_updt_pos(struct ui_persistent *ui);

#endif // UI_PERSISTENT_H