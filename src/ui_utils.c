#include "ui_utils.h"

#include "scenario_select.h"
#include "save_scores.h"

UiState uiState = MAIN_MENU;

ScenarioResults scenarioResults = { 0 };

Font fonts[NUM_FONTS] = { { 0 } };

void initFonts() {
    fonts[FONT_ID_LARGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, LatoBoldTTF_len, 24, NULL, 0);
	SetTextureFilter(fonts[FONT_ID_LARGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_BODY_16] = LoadFontFromMemory(".ttf", LatoRegularTTF, LatoRegularTTF_len, 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_BODY_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, LatoBoldTTF_len, 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_HUGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, LatoBoldTTF_len, 36, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_HUGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_TITLE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, LatoBoldTTF_len, 56, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_TITLE_BOLD].texture, TEXTURE_FILTER_BILINEAR);

    fonts[FONT_ID_GRAPH] = LoadFontFromMemory(".ttf", LatoBoldTTF, LatoBoldTTF_len, 45, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_GRAPH].texture, TEXTURE_FILTER_BILINEAR);

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

void renderCheckbox(bool *checked) {
    CLAY({
        .layout = {
            .sizing = {
                // .width = CLAY_SIZING_FIXED(30),
                // .height = CLAY_SIZING_FIXED(30),
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
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
    *(data->progress) = data->min + percent * (data->max - data->min);
    if (data->snap != 0) {
        float n = *(data->progress) / data->snap;
        int m;
        if ((n - (int) n) >= 0.5f) {
            m = (int) (n + 1.0f);
        } else {
            m = (int) n;
        }
        *(data->progress) = m * data->snap;
    }
}

bool sliderSelected = false;

void handleMoveSlider(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED && !sliderSelected) {
        SliderData *data = (SliderData *) userData;
        data->held = true;
        sliderSelected = true;
    }
}

void renderSlider(SliderData *data, float controlWidth) {
    // TODO: fix this! percents are sometimes negative
    float *progress = data->progress;
    float percent = (*progress - data->min) / (data->max - data->min);
    int id = data->id;
    float leftWidth = percent - controlWidth / 2;
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
            // .border = {
            //     .width = { 3, 3, 3, 3 },
            //     .color = COLOR_BLACK,
            // },
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
            sliderSelected = false;
        }
    }
}

TextBoxData *focusedTextBoxData = NULL;

void handleClickTextBox(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED) {
        TextBoxData *data = (TextBoxData *) userData;
        focusedTextBoxData = data;
        data->focused = true;
    }
}

const char *obfuscatedText =
    "****************"
    "****************"
    "****************"
    "****************"
    "****************"
    "****************"
    "****************"
    "****************";

void renderTextBox(TextBoxData *data) {
    Clay_ElementId textBoxId = CLAY_IDI("textBox", data->id);
    float scrollWidth = Clay_GetScrollContainerData(textBoxId).contentDimensions.width;
    float elementWidth = Clay_GetElementData(textBoxId).boundingBox.width;
    int alignX = CLAY_ALIGN_X_LEFT;
    if (elementWidth < scrollWidth) {
        alignX = CLAY_ALIGN_X_RIGHT;
    }

    if (!data->focused) {
        data->blink = 0.0f;
        data->backspaceDelay = 0.0f;
    } else {
        data->blink += GetFrameTime();
    }

    if (data->focused) {
        int key = GetCharPressed();

        while (key > 0) {
            data->blink = 0.0f;
            if (key < 32 || key > 125) {
                continue;
            }
            if (data->len != TEXT_BOX_MAX_LEN) {
                data->str[data->len] = (char) key;
                data->len++;
            }

            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            data->backspaceDelay = 0.0f;
            data->firstBackspace = true;
        } else {
            data->backspaceDelay = MAX(data->backspaceDelay - GetFrameTime(), 0.0f);
        }

        if (IsKeyDown(KEY_BACKSPACE) && data->len != 0 && FloatEquals(data->backspaceDelay, 0.0f)) {
            data->len--;
            data->str[data->len] = '\0';
            if (data->firstBackspace) {
                data->backspaceDelay = 0.5f;
            } else {
                data->backspaceDelay = 0.02f;
            }
            data->firstBackspace = false;
            data->blink = 0.0f;
        }
    }

    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_PERCENT(1.0),
                .height = CLAY_SIZING_PERCENT(1.0),
            },
            .padding = { 5, 5, 5, 5 },
            .childAlignment = {
                .x = alignX,
                .y = CLAY_ALIGN_Y_CENTER,
            }
        },
        .backgroundColor = COLOR_GRAY,
        .border = {
            .width = { 1, 1, 1, 1 },
            .color = data->focused
                ? (Clay_Color) { 90, 90, 90, 255 }
                : COLOR_LIGHT_GRAY,
        },
        .clip = {
            .horizontal = true,
        },
    }) {
        Clay_OnHover(handleClickTextBox, (intptr_t) data);

        Clay_TextElementConfig textBoxConfig = largeTextConfig;

        if (data->len == 0 && data->placeholder != NULL) {
            textBoxConfig.textColor = COLOR_LIGHT_GRAY;
        } else {
            textBoxConfig.textColor = COLOR_BLANK;
        }
        Clay_String placeholderString = (Clay_String) {
            .isStaticallyAllocated = true,
            .length = data->placeholderLen,
            .chars = data->placeholder,
        };

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .padding = { 5, 5, 5, 5 },
                .childAlignment = {
                    .x = alignX,
                    .y = CLAY_ALIGN_Y_CENTER,
                }
            },
            .clip = {
                .horizontal = true,
            },
            .floating = {
                .attachTo = CLAY_ATTACH_TO_PARENT
            },
        }) {
            Clay_OnHover(handleClickTextBox, (intptr_t) data);
            CLAY_TEXT(placeholderString, CLAY_TEXT_CONFIG(textBoxConfig));
        }

        Clay_String textBoxString = (Clay_String) {
            .isStaticallyAllocated = false,
            .length = data->len,
        };
        if (data->obfuscated) {
            textBoxString.chars = obfuscatedText;
        } else {
            textBoxString.chars = data->str;
        }
        textBoxConfig.textColor = COLOR_WHITE;

        CLAY({
        .id = textBoxId,
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .padding = { 5, 5, 5, 5 },
                .childAlignment = {
                    .x = alignX,
                    .y = CLAY_ALIGN_Y_CENTER,
                }
            },
            .clip = {
                .horizontal = true,
            },
            .floating = {
                .attachTo = CLAY_ATTACH_TO_PARENT
            },
        }) {
            Clay_OnHover(handleClickTextBox, (intptr_t) data);
            CLAY_TEXT(textBoxString, CLAY_TEXT_CONFIG(textBoxConfig));

            if (
                data->focused
                // && data->len != 0
                && (int) (data->blink / 0.5) % 2 == 0
            ) {
            } else {
                textBoxConfig.textColor = COLOR_BLANK;
            }
            CLAY_TEXT(CLAY_STRING("|"), CLAY_TEXT_CONFIG(textBoxConfig));
        }

    }
}

void handleToScenarioSelect(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = SCENARIO_SELECT;
        if (selectedScenarioIndex >= 0) {
            loadSavedScores((*currentFileMetadata)[selectedScenarioIndex]);
        }
    }
}

void handleToSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (loadSettings(SETTINGS_PATH) != 0) {
            fprintf(stderr, "Failed to load settings file upon entering settings menu\n");
        }
        uiState = SETTINGS;
    }
}

void handleToMainMenu(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = MAIN_MENU;
    }
}

void handleToLoginScreen(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uiState = LOGIN;
    }
}
