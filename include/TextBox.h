#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <raylib.h>
#include <stdbool.h>

#include "CONSTANTS.h"

struct TextBox {
    Rectangle rect;
    char input[MAX_INPUT];
    int key;
    int currentCountChar;
    int framesCounter;
    const char* title;
    bool isActive;
    Color colorText;
    Color color;
    Color colorLine;
    Color colorLineActive;
};

void textBoxInit(struct TextBox *TextBox, float posX, float posY, float width, float height, const char* title, Color colorText, Color color, Color colorLine, Color colorLineActive);

void textBoxDraw(struct TextBox *TextBox);

void textBoxGetInput(struct TextBox *TextBox);

void textBoxCheckFocus(struct TextBox *TextBox);

#endif //TEXTBOX_H