/**
 * @file textboxsecret.c
 * @brief Textboxsecret implementation
 */
#include "ui/components/textboxsecret.h"

#include <external/raylib/raygui.h>

struct textboxsecret textboxsecret_init(Rectangle bounds, const char *title) {
    struct textboxsecret tbs = { 0 }; // Initialize struct with zeroed memory
    // inputs already initialized to 0 with above
    tbs.bounds = bounds;
    tbs.secret_view = false;
    tbs.title = title;
    return tbs;
}

void textboxsecret_draw(struct textboxsecret *tbs) {
    // Draw title above the textboxsecret
    GuiLabel((Rectangle) { tbs->bounds.x, tbs->bounds.y - (FONT_SIZE + 5), tbs->bounds.width, 20 }, tbs->title);

    GuiTextBoxSecret(tbs->bounds, tbs->input, MAX_INPUT, &tbs->secret_view);
}
