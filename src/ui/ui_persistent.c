/**
 * @file ui_presistent.c
 * @brief Persisnt ui implementation
 */
#include "ui/ui_persistent.h"

#include <string.h>

#include <external/raylib/raygui.h>

// raygui embedded styles
// Embedded a monospace font in them as well.
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE 15 // NOTE: Included light style
#include "styles/amber.h"           // raygui styleL amber
#include "styles/ashes.h"           // raygui style: ashes
#include "styles/bluish.h"          // raygui style: bluish
#include "styles/candy.h"           // raygui style: candy
#include "styles/cherry.h"          // raygui style: cherry
#include "styles/cyber.h"           // raygui style: cyber
#include "styles/dark.h"            // raygui style: dark
#include "styles/enefete.h"         // raygui style: enefete
#include "styles/genesis.h"         // raygui style: genesis
#include "styles/jungle.h"          // raygui style: jungle
#include "styles/lavanda.h"         // raygui style: lavanda
#include "styles/light.h"           // raygui style: default
#include "styles/rltech.h"          // raygui style: rltech
#include "styles/sunny.h"           // raygui style: sunny
#include "styles/terminal.h"        // raygui style: terminal

#include "globals.h"

void ui_persistent_init(struct ui_persistent *ui) {
    ui->logout_butn = button_init((Rectangle) { window_width - 100, window_height - 60, 0, 30 }, "Log Out");

    ui->ddb_style_options = dropdownbox_init(
        (Rectangle) { window_width - 110, 30, 100, 30 },
        "Amber;Ashes;Bluish;Candy;Cherry;Cyber;Dark;Enefete;Genesis;Jungle;Lavanda;Light;RLTech;Sunny;Terminal",
        "Style:"
    );

    ui->ddb_style_options.active_option = 8;                         ///< Currently active GUI style (default: Genesis)
    ui->prev_active_style = ui->ddb_style_options.active_option - 1; ///< Previously active style for change detection

    ui->statusbar_bounds = (Rectangle) { 0, window_height - 20, window_width, 20 };
}

void ui_persistent_draw(struct ui_persistent *ui, struct user *current_user, enum app_state *state) {
    dropdownbox_draw(&ui->ddb_style_options);

    if (button_draw_updt(&ui->logout_butn)) {
        memset(current_user, 0, sizeof(struct user));
        *state = STATE_LOGIN_MENU;
    }

    GuiStatusBar(
        ui->statusbar_bounds,
        TextFormat("Logged: %s    Current screen: %s", current_user->username, app_state_to_string(state))
    );
}

void ui_persistent_updt(struct ui_persistent *ui) {
    // Handle GUI style changes
    if (ui->ddb_style_options.active_option != ui->prev_active_style) {
        // Reset to default internal style
        // NOTE: Required to unload any previously loaded font texture
        GuiLoadStyleDefault();

        // Load selected style
        switch (ui->ddb_style_options.active_option) {
        case 0: GuiLoadStyleAmber(); break;
        case 1: GuiLoadStyleAshes(); break;
        case 2: GuiLoadStyleBluish(); break;
        case 3: GuiLoadStyleCandy(); break;
        case 4: GuiLoadStyleCherry(); break;
        case 5: GuiLoadStyleCyber(); break;
        case 6: GuiLoadStyleDark(); break;
        case 7: GuiLoadStyleEnefete(); break;
        case 8: GuiLoadStyleGenesis(); break;
        case 9: GuiLoadStyleJungle(); break;
        case 10: GuiLoadStyleLavanda(); break;
        case 11: GuiLoadStyleLight(); break;
        case 12: GuiLoadStyleRLTech(); break;
        case 13: GuiLoadStyleSunny(); break;
        case 14: GuiLoadStyleTerminal(); break;
        default: break;
        }

        ui->prev_active_style = ui->ddb_style_options.active_option;
    }
}

void ui_persistent_updt_pos(struct ui_persistent *ui) {
    ui->logout_butn.bounds.x = window_width - 100;
    ui->logout_butn.bounds.y = window_height - 60;

    ui->ddb_style_options.bounds.x =  window_width - 110;

    ui->statusbar_bounds.y = window_height - 20;
    ui->statusbar_bounds.width = window_width;
}
