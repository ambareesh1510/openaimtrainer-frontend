#ifndef UI_H
#define UI_H

#include "cvector/cvector.h"
#include "tomlc17/tomlc17.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/clay_renderer_raylib.c"

#include "ui_utils.h"
#include "lua_interface.h"
#include "lato.h"

int selectedScenarioIndex = -1;
void handleSelectScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selectedScenarioIndex = (int) userData;
    }
}

cvector_vector_type(ScenarioMetadata) fileMetadata = NULL;

char *timeBuffer = NULL;
void RenderScenarioCard(int index) {
    CLAY({
        .id = CLAY_IDI("ScenarioCard", index),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
            .padding = { 5, 5, 5, 5 },
        },
        .backgroundColor = (index == selectedScenarioIndex)
            ? COLOR_DARK_GREEN
            : (Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY),
    }) {
        ScenarioMetadata metadata = fileMetadata[index];
        Clay_OnHover(handleSelectScenario, index);
        CLAY_TEXT(CLAY_DYNSTR(metadata.name), CLAY_TEXT_CONFIG(boldTextConfig));
        hSpacer();
        CLAY_TEXT(CLAY_DYNSTR(metadata.author), CLAY_TEXT_CONFIG(normalTextConfig));
        hSpacer();

        const char *timeFormat = "%.1fs";
        size_t needed = snprintf(NULL, 0, timeFormat, metadata.time) + 1;
        timeBuffer = realloc(timeBuffer, needed);
        sprintf(timeBuffer, timeFormat, metadata.time);

        CLAY_TEXT(CLAY_DYNSTR(timeBuffer), CLAY_TEXT_CONFIG(normalTextConfig));
    }
}


void findScenarios() {
    selectedScenarioIndex = -1;
    for (size_t i = 0; i < cvector_size(fileMetadata); i++) {
        free(fileMetadata[i].path);
    }
    cvector_clear(fileMetadata);


    // TODO: don't hardcode
    FilePathList dir = LoadDirectoryFiles("../../scenarios");

    // TODO: lazy loading
    for (size_t i = 0; i < dir.count; i++) {
        char *file = dir.paths[i];
        if (IsPathFile(file)) {
            continue;
        }

        FilePathList fileAsDir = LoadDirectoryFiles(file);


        int check = 0;
        char *tomlFilePath = NULL;
        char *luaFilePath = NULL;
        bool fail = false;
        for (size_t j = 0; j < fileAsDir.count; j++) {
            char *scenarioFile = fileAsDir.paths[j];
            const char *scenarioFileName = GetFileName(scenarioFile);
            if (strcmp(scenarioFileName, "info.toml") == 0) {
                tomlFilePath = malloc(strlen(scenarioFile));
                strcpy(tomlFilePath, scenarioFile);
                check++;
            } else if (strcmp(scenarioFileName, "script.lua") == 0) {
                luaFilePath = malloc(strlen(scenarioFile));
                strcpy(luaFilePath, scenarioFile);
                check++;
            }
        }


        UnloadDirectoryFiles(fileAsDir);

        if (check != 2) {
            fail = true;
            goto cleanup;
        }

        toml_result_t tomlParseResult = toml_parse_file_ex(tomlFilePath);
        
        if (!tomlParseResult.ok) {
            fail = true;
            goto cleanup;
        }
        toml_datum_t name = toml_seek(tomlParseResult.toptab, "name");
        toml_datum_t author = toml_seek(tomlParseResult.toptab, "author");
        toml_datum_t description = toml_seek(tomlParseResult.toptab, "description");
        toml_datum_t time = toml_seek(tomlParseResult.toptab, "time");

        // TODO: improve error handling
        if (name.type != TOML_STRING) {
            fprintf(stderr, "Invalid key `name`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        if (author.type != TOML_STRING) {
            fprintf(stderr, "Invalid key `author`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        if (description.type != TOML_STRING) {
            fprintf(stderr, "Invalid key `description`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        if (time.type != TOML_FP64) {
            fprintf(stderr, "Invalid key `time`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        ScenarioMetadata metadata = {
            .path = luaFilePath,
            .name = name.u.s,
            .author = author.u.s,
            .description = description.u.s,
            .time = time.u.fp64
        };

        cvector_push_back(fileMetadata, metadata);

cleanup:
        if (tomlFilePath != NULL) {
            free(tomlFilePath);
        }
        if (fail && (luaFilePath != NULL)) {
            free(luaFilePath);
        }
    }

    UnloadDirectoryFiles(dir);
}

void handleReloadScenarios(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        findScenarios();
    }
}

void handleStartScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        loadLuaScenario(fileMetadata[selectedScenarioIndex]);
    }
}

Clay_RenderCommandArray createScenarioSelectScreen(void) {
    Clay_BeginLayout();
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = { 16, 16, 16, 16 },
            .childGap = 16
        },
        .backgroundColor = COLOR_BLACK,
    }) {
        CLAY({
            .id = CLAY_ID("SideBar"),
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.6),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = { 16, 16, 16, 16 },
                .childGap = 16
            },
            .backgroundColor = COLOR_GRAY,
        }) {
            for (size_t i = 0; i < cvector_size(fileMetadata); i++) {
                RenderScenarioCard(i);
            }
            CLAY({
                .id = CLAY_ID("LeftSpacer"),
                .layout = {
                    .sizing = {
                        .height = CLAY_SIZING_GROW(0)
                    },
                },
            }) {}

            CLAY({
                .id = CLAY_ID("ReloadScenarios"),
                .backgroundColor = Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY,
                .layout = {
                    .padding = { 5, 5, 5, 5 },
                },
            }) {
                Clay_OnHover(handleReloadScenarios, 0);
                CLAY_TEXT(CLAY_STRING("Reload"), &normalTextConfig);
            }
        }

        CLAY({
            .id = CLAY_ID("ScenarioInfo"),
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.4),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = { 16, 16, 16, 16 },
                .childGap = 16
            },
            .backgroundColor = COLOR_GRAY
        }) {
            if (selectedScenarioIndex == -1) {
                CLAY_TEXT(CLAY_STRING("No scenario selected"), CLAY_TEXT_CONFIG(normalTextConfig));
            } else {
                ScenarioMetadata s = fileMetadata[selectedScenarioIndex];
                CLAY_TEXT(
                    CLAY_DYNSTR(s.name),
                    CLAY_TEXT_CONFIG(largeTextConfig)
                );
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                CLAY_TEXT(CLAY_STRING("Author: "), CLAY_TEXT_CONFIG(boldTextConfig));
                CLAY_TEXT(CLAY_DYNSTR(s.author), CLAY_TEXT_CONFIG(normalTextConfig));
                }
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                CLAY_TEXT(CLAY_STRING("Description: "), CLAY_TEXT_CONFIG(boldTextConfig));
                CLAY_TEXT(CLAY_DYNSTR(s.description), CLAY_TEXT_CONFIG(normalTextConfig));
                }
                CLAY({
                    .id = CLAY_ID("RightSpacer"),
                    .layout = {
                        .sizing = {
                            .height = CLAY_SIZING_GROW(0)
                        },
                    },
                }) {}
                CLAY({
                    .id = CLAY_ID("StartScenario"),
                    .backgroundColor = Clay_Hovered()
                        ? COLOR_DARK_BLUE
                        : COLOR_LIGHT_GRAY,
                    .layout = {
                        .padding = { 5, 5, 5, 5 },
                    },
                }) {
                    Clay_OnHover(handleStartScenario, 0);
                    CLAY_TEXT(CLAY_STRING("Start Scenario"), &normalTextConfig);
                }
            }
        }

    }
    return Clay_EndLayout();
}

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

    if (IsKeyPressed(KEY_D)) {
        debugEnabled = !debugEnabled;
        Clay_SetDebugModeEnabled(debugEnabled);
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

    Clay_UpdateScrollContainers(true, (Clay_Vector2) {mouseWheelX, mouseWheelY}, GetFrameTime());
    double currentTime = GetTime();
    Clay_RenderCommandArray renderCommands = createScenarioSelectScreen();
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
}

bool reinitializeClay = false;

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
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

#endif /* UI_H */
