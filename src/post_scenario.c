#include "post_scenario.h"

#include "clay/clay.h"

#include "ui_utils.h"
#include "scenario_select.h"

// TODO: this will be duplicated in the scenario select screen;
//  consider merging them
CustomLayoutElementData progressionGraphData = {
    .type = DRAW_PROGRESSION_GRAPH,
};

CustomLayoutElementData scenarioGraphData = {
    .type = DRAW_SCENARIO_GRAPH,
};

void renderPostScenario(void) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = { 16, 16, 16, 16 },
            .childGap = 8,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = COLOR_GRAY,
    }) {
        CLAY_TEXT(CLAY_DYNSTR((*currentFileMetadata)[selectedScenarioIndex].name), &titleTextConfig);

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(1.0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = 16,
            },
        }) {
            CLAY({
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(1.0),
                        .height = CLAY_SIZING_FIT(0),
                    },
                    .childGap = 16,
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                    },
                },
                // .backgroundColor = COLOR_ORANGE,
            }) {
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(0.3),
                            .height = CLAY_SIZING_FIT(0),
                        },
                        .childGap = 16,
                        .padding = { 8, 8, 8, 8 },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                    },
                    .cornerRadius = CLAY_CORNER_RADIUS(2),
                }) {
                    CLAY_TEXT(CLAY_STRING("Results"), &hugeTextConfig);

                    CDIV(CLAY_SIZING_PERCENT(1.0), CLAY_SIZING_FIT(0)) {
                        CLAY_TEXT(CLAY_STRING("Score"), &largeTextConfig);
                        CDIV(CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)) {}
                        const char *score = TextFormat("%d", scenarioResults.score);
                        CLAY_TEXT(CLAY_DYNSTR(score), &largeTextConfig);
                    }

                    CDIV(CLAY_SIZING_PERCENT(1.0), CLAY_SIZING_FIT(0)) {
                        CLAY_TEXT(CLAY_STRING("Accuracy"), &largeTextConfig);
                        CDIV(CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)) {}
                        const char *accuracy = TextFormat("%.2f%%", scenarioResults.accuracy);
                        CLAY_TEXT(CLAY_DYNSTR(accuracy), &largeTextConfig);
                    }

                }
            }

            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0),
                    },
                    // .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childGap = 16,
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                    },
                },
            }) {
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(0.5),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childGap = 16,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                    },
                }) {
                    CLAY_TEXT(CLAY_STRING("This attempt"), &largeTextConfig);
                    CLAY({
                        .layout = {
                            .sizing = {
                                .height = CLAY_SIZING_GROW(0),
                                .width = CLAY_SIZING_GROW(0),
                            },
                        },
                        .backgroundColor = COLOR_LIGHT_GRAY,
                        .custom = {
                            .customData = &scenarioGraphData,
                        },
                    }) {}
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(0.5),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childGap = 16,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                    },
                }) {
                    CLAY_TEXT(CLAY_STRING("Progress over time"), &largeTextConfig);
                    CLAY({
                        .layout = {
                            .sizing = {
                                .height = CLAY_SIZING_GROW(0),
                                .width = CLAY_SIZING_GROW(0),
                            },
                        },
                        .backgroundColor = COLOR_LIGHT_GRAY,
                        .custom = {
                            .customData = &progressionGraphData,
                        },
                    }) {}
                }
            }
        }

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                },
                .childGap = 10,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                },
                .padding = { 0, 0, 20, 0 },
            },
        }) {
            CLAY({
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.25),
                    },
                    .padding = { 5, 5, 5, 5 },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                    },
                },
            }) {
                Clay_OnHover(handleStartScenario, 0);
                CLAY_TEXT(CLAY_STRING("Replay scenario"), &largeTextConfig);
            }

            CLAY({
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.25),
                    },
                    .padding = { 5, 5, 5, 5 },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                    },
                },
            }) {
                Clay_OnHover(handleToScenarioSelect, 0);
                CLAY_TEXT(CLAY_STRING("Scenario select"), &largeTextConfig);
            }
        }

    }
}
