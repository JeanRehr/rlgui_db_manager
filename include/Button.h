#ifndef BUTTON_H
#define BUTTON_H

#include <raylib.h>
#include <stdbool.h>

struct Button {
    Rectangle rect;
    const char* title;
    bool isPressed;
    Color colorText;
    Color color;
    Color colorLine;
    Color colorActive;
};

void buttonInit(struct Button *button, float posX, float posY, float width, float height, const char* title, Color colorText, Color color, Color colorLine, Color colorActive);

void buttonDraw(struct Button *button);

bool buttonCheckClick(struct Button *button);

#endif //BUTTON_H