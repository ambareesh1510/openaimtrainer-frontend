#ifndef SETTINGS_H
#define SETTINGS_H

#include "ui_utils.h"

bool s = false;
float f = 0.5;
SliderData data = {
    .progress = &f,
    .id = 0,
};

void renderSettingsMenu(void) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = { 16, 16, 16, 16 },
            .childGap = 16,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = COLOR_GRAY,
    }) {
        CLAY_TEXT(CLAY_STRING("Settings"), &titleTextConfig);

        renderCheckbox(&s);
        renderSlider(&data, 0.05);

        const char *sliderPercent = TextFormat("%.2f%%", f * 100.);
        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);

        CLAY({
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(50),
                },
            },
        }) {}

        CLAY({
            .backgroundColor = Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY,
            .layout = {
                .padding = { 5, 5, 5, 5 },
            },
        }) {
            Clay_OnHover(handleToMainMenu, 0);
            CLAY_TEXT(CLAY_STRING("Back"), &hugeTextConfig);
        }
    }
}

#endif /* SETTINGS_H */
