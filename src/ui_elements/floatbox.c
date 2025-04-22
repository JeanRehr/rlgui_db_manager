#include <external/raylib/raygui.h>

#include <stdbool.h>
#include <stddef.h>

#include "ui_elements/floatbox.h"

typedef struct floatbox floatbox;

floatbox floatbox_init(Rectangle bounds, const char *title)
{
    floatbox fb = {0};
    fb.bounds = bounds;
    fb.title = title;
    fb.edit_mode = false;
    fb.value = 0;
	return fb;
}

void floatbox_draw(floatbox *fb)
{
    // Draw title above the floatbox
	GuiLabel((Rectangle){fb->bounds.x, fb->bounds.y - (FONT_SIZE + 5), fb->bounds.width, 20}, fb->title);

	if (GuiValueBoxFloat(fb->bounds, NULL, fb->text_input, &fb->value, fb->edit_mode)) {
		fb->edit_mode = !fb->edit_mode;
	}
}
