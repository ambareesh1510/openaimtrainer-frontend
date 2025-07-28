#include "ui.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#include "main_menu.h"
#include "login_screen.h"
#include "settings.h"
#include "scenario_select.h"
#include "post_scenario.h"
#include "status_bar.h"
#include "lua_interface.h"
#include "save_scores.h"

typedef struct
{
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    bool mouseDown;
} ScrollbarData;

ScrollbarData scrollbarData = {0};

bool debugEnabled = false;

void UpdateDrawFrame(Font* fonts)
{
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;

    if (IsKeyPressed(KEY_TAB)) {
        debugEnabled = !debugEnabled;
        Clay_SetDebugModeEnabled(debugEnabled);
    }

    if (IsMouseButtonDown(0)) {
        if (focusedTextBoxData != NULL) {
            focusedTextBoxData->focused = false;
        }
    }

    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, IsMouseButtonDown(0) && !scrollbarData.mouseDown);
    Clay_SetLayoutDimensions(
        (Clay_Dimensions) {
            (float) GetScreenWidth(),
            (float) GetScreenHeight()
        }
    );
    if (!IsMouseButtonDown(0)) {
        scrollbarData.mouseDown = false;
    }

    Clay_UpdateScrollContainers(false, (Clay_Vector2) {mouseWheelX * 1.5, mouseWheelY * 1.5}, GetFrameTime());
    Clay_BeginLayout();
    CLAY({
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
        },
    }) {
        if (uiState == MAIN_MENU) {
            renderMainMenu();
        } else {
            CDIV(CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)) {
                if (uiState == LOGIN) {
                    renderLoginScreen();
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        uiState = SCENARIO_SELECT;
                    }
                } else if (uiState == SCENARIO_SELECT) {
                    renderScenarioSelectScreen();
                    // TODO: when exiting a scenario using ESC, this causes it to go
                    //  directly to main menu
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        uiState = SCENARIO_SELECT;
                    }
                } else if (uiState == SETTINGS) {
                    renderSettingsMenu();
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        uiState = SCENARIO_SELECT;
                    }
                } else if (uiState == POST_SCENARIO) {
                    renderPostScenario();
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        uiState = SCENARIO_SELECT;
                        if (selectedScenarioIndex >= 0) {
                            loadSavedScores((*currentFileMetadata)[selectedScenarioIndex]);
                        }
                    }
                }
            }
            renderStatusBar();
        }
    }
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
}

bool reinitializeClay = false;

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s\n", errorData.errorText.chars);
    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        reinitializeClay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        reinitializeClay = true;
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }
}

int spawnUi(void) {
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
        totalMemorySize,
        malloc(totalMemorySize)
    );
    Clay_Initialize(
        clayMemory,
        (Clay_Dimensions) {
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        },
        (Clay_ErrorHandler) {
            HandleClayErrors,
            0
        }
    );
    const int screenWidth = 1280;
    const int screenHeight = 720;
    Clay_Raylib_Initialize(
        screenWidth,
        screenHeight,
        "Aim Trainer",
        FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT
    );

    initFonts();

    // TODO: this should really only happen when switching to scenario select
    findScenarios();

    SetExitKey(KEY_NULL);
    while (!WindowShouldClose()) {
        if (reinitializeClay) {
            Clay_SetMaxElementCount(8192);
            totalMemorySize = Clay_MinMemorySize();
            clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
            Clay_Initialize(
                clayMemory,
                (Clay_Dimensions) {
                    (float)GetScreenWidth(),
                    (float)GetScreenHeight()
                },
                (Clay_ErrorHandler) {
                    HandleClayErrors,
                    0
                }
            );
            reinitializeClay = false;
        }
        UpdateDrawFrame(fonts);
    }

    Clay_Raylib_Close();
    return 0;
}
