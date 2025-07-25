#include "login_screen.h"

#include "clay/clay.h"
#include "ui_utils.h"

void handleSignUp(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        printf("Sign up\n");
    }
}

void handleLogIn(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        printf("Log in\n");
    }
}

#define USERNAME_PLACEHOLDER "Username"
TextBoxData usernameData = {
    .placeholder = USERNAME_PLACEHOLDER,
    .placeholderLen = sizeof(USERNAME_PLACEHOLDER) - 1,
    .id = 1,
};

#define PASSWORD_PLACEHOLDER "Password"
TextBoxData passwordData = {
    .placeholder = PASSWORD_PLACEHOLDER,
    .placeholderLen = sizeof(PASSWORD_PLACEHOLDER) - 1,
    .obfuscated = true,
    .id = 2,
};

void renderLoginScreen(void) {
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
        CLAY_TEXT(CLAY_STRING("Log In"), &titleTextConfig);

        CLAY({
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(50),
                },
            },
        }) {}

        CDIV(CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(50)) {
            renderTextBox(&usernameData);
        }

        CDIV(CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(50)) {
            renderTextBox(&passwordData);
        }

        CLAY({
            .backgroundColor = Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY,
            .layout = {
                .padding = { 5, 5, 5, 5 },
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3),
                },
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                },
            },
        }) {
            Clay_OnHover(handleSignUp, 0);
            CLAY_TEXT(CLAY_STRING("Sign up"), &hugeTextConfig);
        }

        CLAY({
            .backgroundColor = Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY,
            .layout = {
                .padding = { 5, 5, 5, 5 },
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3),
                },
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                },
            },
        }) {
            Clay_OnHover(handleLogIn, 0);
            CLAY_TEXT(CLAY_STRING("Log In"), &hugeTextConfig);
        }
    }
}
