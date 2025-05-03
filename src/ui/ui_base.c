#include "ui/ui_base.h"

#include <stdio.h>

static void ui_default_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    printf("RENDER NOT IMPLEMENTED FOR THIS SCREEN\n");
}

static void
ui_default_handle_buttons(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    printf("HANDLE BUTTONS NOT IMPLEMENTED FOR THIS SCREEN\n");
}

static void
ui_default_handle_warning_msg(struct ui_base *base, enum app_state *state, enum error_code *error, database *db) {
    (void)base;
    (void)state;
    (void)error;
    (void)db;
    printf("HANDLE WARNING MSGS NOT IMPLEMENTED FOR THIS SCREEN\n");
}

static void ui_default_update_positions(struct ui_base *base) {
    (void)base;
    printf("UPDATE POSITIONS NOT IMPLEMENTED FOR THIS SCREEN\n");
}

static void ui_default_clear_fields(struct ui_base *base) {
    (void)base;
    printf("CLEAR FIELDS NOT IMPLEMENTED FOR THIS SCREEN\n");
}

void ui_base_init_defaults(struct ui_base *base) {
    *base = (struct ui_base) {
        .render = ui_default_render,
        .handle_buttons = ui_default_handle_buttons,
        .handle_warning_msg = ui_default_handle_warning_msg,
        .update_positions = ui_default_update_positions,
        .clear_fields = ui_default_clear_fields,
    };
}