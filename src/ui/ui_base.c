/**
 * @file ui_base.c
 * @brief Ui base interface implementation (just stubs)
 */

#include "ui/ui_base.h"

#include <stdio.h>

static void ui_default_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    fprintf(stderr, "Render not implemented for [%s]\n", base->type_name);
}

static void
ui_default_handle_buttons(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    fprintf(stderr, "Handle buttons not implemented for [%s]\n", base->type_name);
}

static void
ui_default_handle_warning_msg(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    fprintf(stderr, "Handle warning msgs not implemented for [%s]\n", base->type_name);
}

static void ui_default_update_positions(struct ui_base *base) {
    (void)base;
    fprintf(stderr, "Update positions not implemented for [%s]\n", base->type_name);
}

static void ui_default_clear_fields(struct ui_base *base) {
    (void)base;
    fprintf(stderr, "Clear fields not implemented for [%s]\n", base->type_name);
}

static void ui_default_cleanup(struct ui_base *base) {
    (void)base;
    fprintf(stderr, "Cleanup not implemented for [%s]\n", base->type_name);
}

void ui_base_init_defaults(struct ui_base *base, const char *type_name) {
    *base = (struct ui_base) {
        .type_name = type_name,
        .render = ui_default_render,
        .handle_buttons = ui_default_handle_buttons,
        .handle_warning_msg = ui_default_handle_warning_msg,
        .update_positions = ui_default_update_positions,
        .clear_fields = ui_default_clear_fields,
        .cleanup = ui_default_cleanup,
    };
}
