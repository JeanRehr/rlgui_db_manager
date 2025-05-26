/**
 * @file ui_clothes.c
 * @brief Clothes screen implementation
 */
#include "ui/screens/ui_clothes.h"

#include <limits.h> // For INT_MAX
#include <stddef.h>
#include <stdio.h> // temporary

#include <external/raylib/raygui.h>

#include "db/clothes_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_clothes_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *clothes_db
);

static void ui_clothes_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *clothes_db
);

static void ui_clothes_update_positions(struct ui_base *base);

static void handle_back_button(enum app_state *state);

static void handle_insert_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db);

static void handle_remove_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db);

static void handle_view_all_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_clothes_init(struct ui_clothes *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Clothes");
    // Override methods
    ui->base.render = ui_clothes_render;
    ui->base.handle_buttons = ui_clothes_handle_buttons;
    ui->base.update_positions = ui_clothes_update_positions;

    // Initialize ui specific fields

    ui->lv_type = listview_init(
        (Rectangle) { 20, 100, 130, 155 },
        "Clothing type:",
        "Tshirt;Shirts;Jeans;Pants;Shorts;Skirts;Dresses;Sweaters;Hoodies;Jackets;Coats;Blazers;Socks;Underwear;Swimwear;Activewear;Pajamas;Hats;Scarves;Gloves;Suits;Vests;Boots;Sneakers;Sandals;Other type"
    );

    ui->lv_size = listview_init(
        (Rectangle) { 20, ui->lv_type.bounds.y + ui->lv_type.bounds.height + (FONT_SIZE * 2), 130, 155 },
        "Size:",
        "XXS;XS;S;M;L;XL;XXL"
    );

    ui->lv_color = listview_init(
        (Rectangle) { 20, ui->lv_size.bounds.y + ui->lv_size.bounds.height + (FONT_SIZE * 2), 130, 155 },
        "Primary color:",
        "Black;White;Gray;Blue;Red;Green;Yellow;Brown;Pink;Purple;Orange;Multicolored;Patterned;Other color"
    );

    ui->ddb_gender = dropdownbox_init(
        (Rectangle) { ui->lv_type.bounds.x + ui->lv_type.bounds.width + 20, 100, 130, 30 },
        "Unissex;Male;Female",
        "Gender:"
    );

    ui->ddb_condition = dropdownbox_init(
        (Rectangle) { ui->ddb_gender.bounds.x,
                      ui->ddb_gender.bounds.y + ui->ddb_gender.bounds.height + (FONT_SIZE * 2),
                      130,
                      30 },
        "New;Good;Worn;Needs repair",
        "Condition:"
    );

    ui->ib_quantity = intbox_init(
        (Rectangle) { ui->ddb_condition.bounds.x,
                      ui->ddb_condition.bounds.y + ui->ddb_condition.bounds.height + (FONT_SIZE * 2),
                      130,
                      30 },
        "Quantity:",
        0,
        INT_MAX
    );

    ui->tb_notes = textbox_init(
        (Rectangle) { ui->ib_quantity.bounds.x,
                      ui->ib_quantity.bounds.y + ui->ib_quantity.bounds.height + (FONT_SIZE * 2),
                      300,
                      30 },
        "Notes:"
    );

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->butn_insert = button_init((Rectangle) { 20, window_height - 60, 100, 30 }, "Insert");

    ui->butn_remove = button_init(
        (Rectangle) { ui->butn_insert.bounds.x + ui->butn_insert.bounds.width + 10, window_height - 60, 100, 30 },
        "Remove"
    );

    ui->butn_view_all = button_init(
        (Rectangle) { ui->butn_remove.bounds.x + ui->butn_remove.bounds.width + 10, window_height - 60, 100, 30 },
        "View All"
    );

    ui->sp_table_view = scrollpanel_init(
        (Rectangle) { ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 10,
                      10,
                      window_width - (ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 20),
                      window_height - 100 },
        "Database view",
        (Rectangle) { 0, 0, 0, 0 }
    );

    ui->str_table_content = NULL;

    ui->flag = 0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Clothes screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param clothes_db Pointer to the clothes database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_clothes_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *clothes_db
) {
    struct ui_clothes *ui = (struct ui_clothes *)base;

    // Dropdowns needs to be first
    dropdownbox_draw(&ui->ddb_gender);
    dropdownbox_draw(&ui->ddb_condition);

    listview_draw(&ui->lv_type);
    listview_draw(&ui->lv_size);
    listview_draw(&ui->lv_color);

    intbox_draw(&ui->ib_quantity);
    textbox_draw(&ui->tb_notes);

    scrollpanel_draw(&ui->sp_table_view, NULL, ui->str_table_content);

    ui->base.handle_buttons(&ui->base, state, error, clothes_db);
}

/**
 * @brief Handle button drawing and logic.
 * 
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param clothes_db Pointer to clothes database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_clothes_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *clothes_db
) {
    (void)clothes_db;

    struct ui_clothes *ui = (struct ui_clothes *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(state);
        return;
    }

    if (button_draw_updt(&ui->butn_insert)) {
        handle_insert_button(ui, error, clothes_db);
        return;
    }

    if (button_draw_updt(&ui->butn_remove)) {
        handle_remove_button(ui, error, clothes_db);
        return;
    }

    if (button_draw_updt(&ui->butn_view_all)) {
        handle_view_all_button(ui, error, clothes_db);
        return;
    }
}

static void ui_clothes_update_positions(struct ui_base *base) {
    struct ui_clothes *ui = (struct ui_clothes *)base;

    ui->butn_insert.bounds.y = window_height - 60;
    ui->butn_remove.bounds.y = window_height - 60;
    ui->butn_view_all.bounds.y = window_height - 60;

    ui->sp_table_view.panel_bounds.width = window_width - (ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 20);
    ui->sp_table_view.panel_bounds.height = window_height - 100;
}

/** @} */

/* ======================= INTERNAL HELPERS ======================= */

static void handle_back_button(enum app_state *state) {
    *state = STATE_MAIN_MENU;
}

static void handle_insert_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db) {
    (void)ui;
    (void)error;
    (void)clothes_db;
    printf("Insert button not implemented.\n");
}

static void handle_remove_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db) {
    (void)ui;
    (void)error;
    (void)clothes_db;
    printf("Remove button not implemented.\n");
}

static void handle_view_all_button(struct ui_clothes *ui, enum error_code *error, database *clothes_db) {
    (void)ui;
    (void)error;
    (void)clothes_db;
    printf("View all button not implemented.\n");
}
