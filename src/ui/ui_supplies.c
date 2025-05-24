/**
 * @file ui_supplies.c
 * @brief Supplies screen implementation
 */
#include "ui/ui_supplies.h"

#include <external/raylib/raygui.h>

#include "db/supplies_db.h"
#include "globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_supplies_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *supplies_db);

static void ui_supplies_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
);

static void handle_back_button(enum app_state *state);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_supplies_init(struct ui_supplies *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Supplies");
    // Override methods
    ui->base.render = ui_supplies_render;
    ui->base.handle_buttons = ui_supplies_handle_buttons;

    // Initialize ui specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->flag = 0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Supplies screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param supplies_db Pointer to the supplies database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_supplies_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
) {
    struct ui_supplies *ui = (struct ui_supplies *)base;

    ui->base.handle_buttons(&ui->base, state, error, supplies_db);
}

/**
 * @brief Handle button drawing and logic.
 * 
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param supplies_db Pointer to supplies database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_supplies_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
) {
    (void)error;
    (void)supplies_db;

    struct ui_supplies *ui = (struct ui_supplies *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(state);
        return;
    }
}
/** @} */

/* ======================= INTERNAL HELPERS ======================= */

static void handle_back_button(enum app_state *state) {
    *state = STATE_MAIN_MENU;
}
