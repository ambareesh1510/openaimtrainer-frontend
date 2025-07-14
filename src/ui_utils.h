#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <stdbool.h>

#include "lato.h"

enum {
    MAIN_MENU,
    SCENARIO_SELECT,
    SETTINGS,
    POST_SCENARIO,
} uiState = MAIN_MENU;

struct {
    bool valid;
    int score;
    float accuracy;
} scenarioResults;

// TODO: move this to a more appropriate place
struct ScenarioMetadata {
    char *path;
    const char *name;
    const char *author;
    const char *description;
    double time;
};
typedef struct ScenarioMetadata ScenarioMetadata;

#define COLOR_BLACK (Clay_Color) {0, 0, 0, 255}
#define COLOR_GRAY (Clay_Color) {30, 30, 30, 255}
#define COLOR_LIGHT_GRAY (Clay_Color) {50, 50, 50, 255}
#define COLOR_DARK_BLUE (Clay_Color) {50, 50, 80, 255}
#define COLOR_DARK_GREEN (Clay_Color) {50, 80, 50, 255}
#define COLOR_WHITE (Clay_Color) {255, 255, 255, 255}
#define COLOR_ORANGE (Clay_Color) {225, 138, 50, 255}
#define COLOR_BLUE (Clay_Color) {111, 173, 162, 255}

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) (Clay_Vector2) { .x = vector.x, .y = vector.y }

#define CLAY_DYNSTR(s) ((Clay_String) { .chars = s, .length = strlen(s) })

enum {
    FONT_ID_LARGE_BOLD,
    FONT_ID_BODY_16,
    FONT_ID_BODY_BOLD,
    FONT_ID_HUGE_BOLD,
    FONT_ID_TITLE_BOLD,
    NUM_FONTS,
};

Font fonts[NUM_FONTS];

void initFonts() {
    fonts[FONT_ID_LARGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 24, NULL, 0);
	SetTextureFilter(fonts[FONT_ID_LARGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_BODY_16] = LoadFontFromMemory(".ttf", LatoRegularTTF, sizeof(LatoRegularTTF), 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_BODY_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_HUGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 36, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_HUGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_TITLE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 56, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_TITLE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
}

Clay_TextElementConfig largeTextConfig = {
    .fontId = FONT_ID_LARGE_BOLD,
    .letterSpacing = 1,
    .fontSize = 24,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig normalTextConfig = {
    .fontId = FONT_ID_BODY_16,
    .letterSpacing = 1,
    .fontSize = 16,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig boldTextConfig = {
    .fontId = FONT_ID_BODY_BOLD,
    .letterSpacing = 1,
    .fontSize = 16,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig hugeTextConfig = {
    .fontId = FONT_ID_HUGE_BOLD,
    .letterSpacing = 1,
    .fontSize = 36,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig titleTextConfig = {
    .fontId = FONT_ID_TITLE_BOLD,
    .letterSpacing = 1,
    .fontSize = 56,
    .textColor = COLOR_WHITE
};

void hSpacer() {
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {};
}

void handleClickCheckbox(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        bool *checked = (bool *) userData;
        *checked = !(*checked);
    }
}

// TODO: make the checkbox grow so that caller can define dimensions?
void renderCheckbox(bool *checked) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(30),
                .height = CLAY_SIZING_FIXED(30),
            },
        },
        .border = {
            .width = { 5, 5, 5, 5 },
            .color = COLOR_WHITE,
        },
        .backgroundColor =
            (*checked)
                ? COLOR_WHITE
                : (Clay_Color) { 0, 0, 0, 0 }, 0.05,
    }) {
        Clay_OnHover(handleClickCheckbox, (intptr_t) checked);
    }
}

// TODO: change progress from (float *) to float
// TODO: remove ID field from this struct
struct SliderData {
    float *progress;
    int id;
    bool held;
};
typedef struct SliderData SliderData;

void updateSlider(Clay_ElementId elementId, Clay_Vector2 pointerPosition, SliderData *data) {
    Clay_ElementData sliderData = Clay_GetElementData(elementId);
    float totalWidth = sliderData.boundingBox.width;
    float currentWidth = pointerPosition.x - sliderData.boundingBox.x;
    float percent = currentWidth / totalWidth;
    if (percent < 0.) {
        percent = 0.;
    }
    if (percent > 1.) {
        percent = 1.;
    }
    *(data->progress) = percent;
}

void handleMoveSlider(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED) {
        SliderData *data = (SliderData *) userData;
        data->held = true;
    }
}

void renderSlider(SliderData *data, float controlWidth) {
    // TODO: fix this! percents are sometimes negative
    float *progress = data->progress;
    int id = data->id;
    float leftWidth = *progress - controlWidth / 2;
    float rightWidth = 1.0 - controlWidth - leftWidth;
    Clay_ElementId sliderId = CLAY_IDI("slider", id);

    CLAY({
        .id = sliderId,
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(300),
                .height = CLAY_SIZING_FIXED(30),
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
    }) {
        Clay_OnHover(handleMoveSlider, (intptr_t) data);
        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(leftWidth),
                    .height = CLAY_SIZING_FIXED(10),
                },
            },
            .backgroundColor = COLOR_BLUE,
        }) {
        }
        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(controlWidth),
                    .height = CLAY_SIZING_FIXED(30),
                },
            },
            .border = {
                .width = { 3, 3, 3, 3 },
                .color = COLOR_BLACK,
            },
            .backgroundColor = COLOR_WHITE,
        }) {
        }
        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(rightWidth),
                    .height = CLAY_SIZING_FIXED(10),
                },
            },
            .backgroundColor = COLOR_ORANGE,
        }) {
        }
    }
    if (data->held) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 raylibPointerPosition = GetMousePosition();
            Clay_Vector2 pointerPosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(raylibPointerPosition);
            updateSlider(sliderId, pointerPosition, data);
        } else {
            data->held = false;
        }
    }
}


void handleToScenarioSelect(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = SCENARIO_SELECT;
    }
}

void handleToSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = SETTINGS;
    }
}

void handleToMainMenu(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = MAIN_MENU;
    }
}

#endif /* UI_UTILS_H */
