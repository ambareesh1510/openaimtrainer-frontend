#include "status_bar.h"

#include "clay/clay.h"

#include "ui_utils.h"
#include "network.h"

Clay_TextElementConfig usernameTextConfig = {
    .fontId = FONT_ID_LARGE_BOLD,
    .letterSpacing = 1,
    .fontSize = 24,
    .textColor = COLOR_BLUE,
};

Clay_ElementDeclaration statusBarLeftElementConfig = {
    .border = {
        .width = { 2, 0, 0, 0, },
        .color = COLOR_GRAY,
    },
    .layout = {
        .padding = { 15, 15, 10, 10 },
    },
};

Clay_ElementDeclaration statusBarRightElementConfig = {
    // .backgroundColor = Clay_Hovered() ? COLOR_BLUE : COLOR_ORANGE,
    .border = {
        .width = { 0, 2, 0, 0, },
        .color = COLOR_GRAY,
    },
    .layout = {
        .padding = { 15, 15, 10, 10 },
    },
};


void renderStatusBar(void) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = COLOR_DARK_GRAY,
    }) {
        CLAY({
            .backgroundColor = Clay_Hovered() ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY,
            .border = {
                .width = { 2, 0, 0, 0, },
                .color = COLOR_GRAY,
            },
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIT(0),
                },
                .padding = { 15, 15, 10, 10 },
                .childGap = 15,
            },
        }) {
            if (username != NULL) {
                CLAY_TEXT(CLAY_STRING("Signed in as "), &largeTextConfig);
                CLAY_TEXT(CLAY_DYNSTR(username), &usernameTextConfig);
            } else {
                Clay_OnHover(handleToLoginScreen, 0);
                CLAY_TEXT(CLAY_STRING("Not signed in"), &largeTextConfig);
            }
        }

        CDIV(CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)) {}

        CLAY({
            .backgroundColor = Clay_Hovered() ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY,
            .border = {
                .width = { 0, 2, 0, 0, },
                .color = COLOR_GRAY,
            },
            .layout = {
                .padding = { 15, 15, 10, 10 },
            },
        }) {
            Clay_OnHover(handleToSettings, 0);
            CLAY_TEXT(CLAY_STRING("Settings"), &largeTextConfig);
        }
    }
}
