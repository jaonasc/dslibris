#ifndef DSLIBRIS_UI_H
#define DSLIBRIS_UI_H
#include "text.h"
#include "reader_visual.h"

// Restore all text state after drawing menus, without changing reading settings.
struct MenuScope {
    Text* t;
    u16* screen;
    u16 paper, ink, x, y;
    u8 size;
    int style;
    MenuScope(Text* text, u16* target, int theme);
    ~MenuScope();
};
void UiText(Text* t, int x, int baseline, const char* text, int width, int size, u16 ink);
void UiHeader(Text* t, const char* title, const char* subtitle, int theme);
const char* ThemeName(int theme);
#endif
