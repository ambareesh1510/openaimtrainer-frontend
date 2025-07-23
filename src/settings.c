#include "settings.h"

void handleResetCrosshairSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        currentCrosshairConfig = (CrosshairConfig) DEFAULT_CROSSHAIR_CONFIG;
    }
}

void handleResetSensitivity(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        sensitivity = DEFAULT_SENSITIVITY;
    }
}

void handleExitSettingsMenu(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (loadSettings(SETTINGS_PATH) != 0) {
            fprintf(stderr, "Failed to load settings file upon exiting settings menu\n");
            currentCrosshairConfig = (CrosshairConfig) DEFAULT_CROSSHAIR_CONFIG;
        }
        uiState = MAIN_MENU;
    }
}

void handleSaveSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        saveSettings(SETTINGS_PATH);
    }
}

SliderData centerDotRadiusSliderData = {
    .progress = &currentCrosshairConfig.centerDotRadius,
    .id = 0,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData innerLineLengthSliderData = {
    .progress = &currentCrosshairConfig.innerLineLength,
    .id = 1,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData innerLineWidthSliderData = {
    .progress = &currentCrosshairConfig.innerLineWidth,
    .id = 2,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData innerLineGapSliderData = {
    .progress = &currentCrosshairConfig.innerLineGap,
    .id = 3,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData outerLineLengthSliderData = {
    .progress = &currentCrosshairConfig.outerLineLength,
    .id = 4,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData outerLineWidthSliderData = {
    .progress = &currentCrosshairConfig.outerLineWidth,
    .id = 5,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData outerLineGapSliderData = {
    .progress = &currentCrosshairConfig.outerLineGap,
    .id = 6,
    .min = 0.0f,
    .max = 10.0f,
    .snap = 1.0f,
};

SliderData sensitivitySliderData = {
    .progress = &sensitivity,
    .id = 7,
    .min = 0.01f,
    .max = 2.0f,
};

CustomLayoutElementData settingsCrosshairData = {
    .type = DRAW_CROSSHAIR_TEXTURE,
};

void renderSettingsMenu(void) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                // .height = CLAY_SIZING_GROW(0)
            },
            .padding = { 16, 16, 16, 16 },
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = COLOR_GRAY,
        .clip = {
            .vertical = true,
            .childOffset = Clay_GetScrollOffset(),
        },
    }) {
        HCENTER {
            CLAY_TEXT(CLAY_STRING("Settings"), &titleTextConfig);
        }

        HCENTER {
            CLAY_TEXT(CLAY_STRING("Crosshair"), &hugeTextConfig);
        }

        CLAY({
            .id = CLAY_ID("CrosshairContainer"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
            },
        }) {
            CLAY({
                .layout = {
                    .padding = { 5, 5, 5, 5 },
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.5),
                    },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            }) {
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Center dot"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        CDIV(CLAY_SIZING_FIXED(20), CLAY_SIZING_FIXED(20)) {
                            renderCheckbox(&currentCrosshairConfig.centerDot);
                        }
                        if (currentCrosshairConfig.centerDot) {
                            CLAY_TEXT(CLAY_STRING("Enabled"), &largeTextConfig);
                        } else {
                            CLAY_TEXT(CLAY_STRING("Disabled"), &largeTextConfig);
                        }
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Center dot radius"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&centerDotRadiusSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.centerDotRadius);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .height = CLAY_SIZING_FIXED(10),
                        },
                    },
                }) {}

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Inner lines"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        CDIV(CLAY_SIZING_FIXED(20), CLAY_SIZING_FIXED(20)) {
                            renderCheckbox(&currentCrosshairConfig.innerLines);
                        }
                        if (currentCrosshairConfig.innerLines) {
                            CLAY_TEXT(CLAY_STRING("Enabled"), &largeTextConfig);
                        } else {
                            CLAY_TEXT(CLAY_STRING("Disabled"), &largeTextConfig);
                        }
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Inner line length"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&innerLineLengthSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.innerLineLength);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Inner line width"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&innerLineWidthSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.innerLineWidth);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Inner line gap"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&innerLineGapSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.innerLineGap);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .height = CLAY_SIZING_FIXED(10),
                        },
                    },
                }) {}

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Outer lines"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        CDIV(CLAY_SIZING_FIXED(20), CLAY_SIZING_FIXED(20)) {
                            renderCheckbox(&currentCrosshairConfig.outerLines);
                        }
                        if (currentCrosshairConfig.outerLines) {
                            CLAY_TEXT(CLAY_STRING("Enabled"), &largeTextConfig);
                        } else {
                            CLAY_TEXT(CLAY_STRING("Disabled"), &largeTextConfig);
                        }
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Outer line length"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&outerLineLengthSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.outerLineLength);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Outer line width"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&outerLineWidthSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.outerLineWidth);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                        CLAY_TEXT(CLAY_STRING("Outer line gap"), &largeTextConfig);
                    }
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                        renderSlider(&outerLineGapSliderData, 0.05);
                        const char *sliderPercent = TextFormat("%.2f", currentCrosshairConfig.outerLineGap);
                        CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                    }
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .height = CLAY_SIZING_FIXED(10),
                        },
                    },
                }) {}

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                        },
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    CLAY({
                        .backgroundColor = Clay_Hovered()
                            ? COLOR_DARK_BLUE
                            : COLOR_LIGHT_GRAY,
                        .layout = {
                            .padding = { 5, 5, 5, 5 },
                        },
                    }) {
                        Clay_OnHover(handleResetCrosshairSettings, 0);
                        CLAY_TEXT(CLAY_STRING("Reset all crosshair settings to default values"), &largeTextConfig);
                    }
                }

            }

            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.5),
                        .height = CLAY_SIZING_GROW(0),
                        // .height = CLAY_SIZING_FIXED(500),
                    },
                    .childGap = 16,
                },
                .backgroundColor = COLOR_LIGHT_GRAY,
                .custom = {
                    .customData = &settingsCrosshairData,
                },
            }) {
            }
        }

        HCENTER {
            CLAY_TEXT(CLAY_STRING("Sensitivity"), &hugeTextConfig);
        }

        CLAY({
            .id = CLAY_ID("SensitivityContainer"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.5),
                    // .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = 16,
            },
        }) {
            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                    },
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childGap = 16,
                },
            }) {
                CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.3), }, }, }) {
                    CLAY_TEXT(CLAY_STRING("Sensitivity"), &largeTextConfig);
                }
                CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.7), }, .childGap = 16, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, }, }, }) {
                    renderSlider(&sensitivitySliderData, 0.05);
                    const char *sliderPercent = TextFormat("%.2f", sensitivity);
                    CLAY_TEXT(CLAY_DYNSTR(sliderPercent), &largeTextConfig);
                }
            }

            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                    },
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childGap = 16,
                },
            }) {
                CLAY({
                    .backgroundColor = Clay_Hovered()
                        ? COLOR_DARK_BLUE
                        : COLOR_LIGHT_GRAY,
                    .layout = {
                        .padding = { 5, 5, 5, 5 },
                    },
                }) {
                    Clay_OnHover(handleResetSensitivity, 0);
                    CLAY_TEXT(CLAY_STRING("Reset sensitivity to default value"), &largeTextConfig);
                }
            }
        }

        CLAY({
            .layout = {
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childGap = 16,
            },
        }) {
            CLAY({
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .padding = { 5, 5, 5, 5 },
                },
            }) {
                Clay_OnHover(handleExitSettingsMenu, 0);
                CLAY_TEXT(CLAY_STRING("Back"), &hugeTextConfig);
            }

            CLAY({
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .padding = { 5, 5, 5, 5 },
                },
            }) {
                Clay_OnHover(handleSaveSettings, 0);
                CLAY_TEXT(CLAY_STRING("Save"), &hugeTextConfig);
            }
        }
    }
}
