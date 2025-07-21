#include "main_menu.h"

void renderMainMenu(void) {
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
        CLAY_TEXT(CLAY_STRING("Aim Trainer"), &titleTextConfig);

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
            Clay_OnHover(handleToScenarioSelect, 0);
            CLAY_TEXT(CLAY_STRING("Scenario Select"), &hugeTextConfig);
        }

        CLAY({
            .backgroundColor = Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY,
            .layout = {
                .padding = { 5, 5, 5, 5 },
            },
        }) {
            Clay_OnHover(handleToSettings, 0);
            CLAY_TEXT(CLAY_STRING("Settings"), &hugeTextConfig);
        }
    }
}
