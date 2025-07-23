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
            .childGap = 16,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = COLOR_GRAY,
    }) {
        CLAY_TEXT(CLAY_STRING("Scenario complete"), &titleTextConfig);

        const char *score = TextFormat("Score: %d", scenarioResults.score);
        CLAY_TEXT(CLAY_DYNSTR(score), &hugeTextConfig);

        const char *accuracy = TextFormat("Accuracy: %.2f%%", scenarioResults.accuracy);
        CLAY_TEXT(CLAY_DYNSTR(accuracy), &hugeTextConfig);

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childGap = 16,
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

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                },
                .childGap = 16,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
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
                Clay_OnHover(handleStartScenario, 0);
                CLAY_TEXT(CLAY_STRING("Replay scenario"), &largeTextConfig);
            }

            CLAY({
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .padding = { 5, 5, 5, 5 },
                },
            }) {
                Clay_OnHover(handleToScenarioSelect, 0);
                CLAY_TEXT(CLAY_STRING("Scenario select"), &largeTextConfig);
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
                CLAY_TEXT(CLAY_STRING("Settings"), &largeTextConfig);
            }
        }

    }
}
