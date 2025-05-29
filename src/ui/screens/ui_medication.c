/**
 * @file ui_medication.c
 * @brief Medication screen implementation
 */
#include "ui/screens/ui_medication.h"

#include <limits.h> // For INT_MAX

#include <external/raylib/raygui.h>

#include "db/medication_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_medication_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
);

static void ui_medication_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
);

static void handle_back_button(enum app_state *state);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_medication_init(struct ui_medication *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Medication");
    // Override methods
    ui->base.render = ui_medication_render;
    ui->base.handle_buttons = ui_medication_handle_buttons;

    // Initialize ui specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_name = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + ui->butn_back.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Medication Name:"
    );
    ui->tb_generic_name = textbox_init(
        (Rectangle) { 20, ui->tb_name.bounds.y + ui->tb_name.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Generic Name:"
    );
    ui->tb_form = textbox_init(
        (Rectangle) { 20, ui->tb_generic_name.bounds.y + ui->tb_generic_name.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Form:"
    );
    ui->tb_strength = textbox_init(
        (Rectangle) { 20, ui->tb_form.bounds.y + ui->tb_form.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Strength:"
    );
    ui->tb_unit = textbox_init(
        (Rectangle) { 20, ui->tb_strength.bounds.y + ui->tb_strength.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Unit:"
    );
    ui->ib_stock = intbox_init(
        (Rectangle) { 20, ui->tb_unit.bounds.y + ui->tb_unit.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Stock:",
        0,
        INT_MAX
    );

    ui->expirationDateText = (Rectangle) { 20,
                                           ui->ib_stock.bounds.y + ui->ib_stock.bounds.height + (FONT_SIZE * 2),
                                           MeasureText("Expiration date:", FONT_SIZE),
                                           20 };

    ui->ib_year = intbox_init(
        (Rectangle) { 20, ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2), 40, 30 },
        "Year",
        0,
        9999
    );

    ui->ib_month = intbox_init(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5,
                      ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Month",
        0,
        12
    );

    ui->ib_day = intbox_init(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5,
                      ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Day",
        0,
        31
    );

    ui->tb_notes = textbox_init(
        (Rectangle) { 20, ui->ib_year.bounds.y + ui->ib_year.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "General notes:"
    );

    ui->flag = 0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Medication screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param medication_db Pointer to the medication database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_medication_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
) {
    struct ui_medication *ui = (struct ui_medication *)base;

    textbox_draw(&ui->tb_name);
    textbox_draw(&ui->tb_generic_name);
    textbox_draw(&ui->tb_form);
    textbox_draw(&ui->tb_strength);
    textbox_draw(&ui->tb_unit);
    intbox_draw(&ui->ib_stock);

    GuiLabel(ui->expirationDateText, "Expiration date:");

    intbox_draw(&ui->ib_year);
    GuiLabel(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width - 1,
                      ui->ib_year.bounds.y + (ui->ib_year.bounds.height / 2) - 5,
                      10,
                      10 },
        "-"
    );
    intbox_draw(&ui->ib_month);
    GuiLabel(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width - 1,
                      ui->ib_month.bounds.y + (ui->ib_month.bounds.height / 2) - 5,
                      10,
                      10 },
        "-"
    );
    intbox_draw(&ui->ib_day);

    textbox_draw(&ui->tb_notes);

    ui->base.handle_buttons(&ui->base, state, error, medication_db);

    if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
    }
}

/**
 * @brief Handle button drawing and logic.
 * 
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param medication_db Pointer to medication database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_medication_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
) {
    (void)error;
    (void)medication_db;

    struct ui_medication *ui = (struct ui_medication *)base;

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
