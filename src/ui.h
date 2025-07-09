#ifndef UI_H
#define UI_H

#include "tinydir/tinydir.h"
#include "cvector/cvector.h"
#include "tomlc17/tomlc17.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/clay_renderer_raylib.c"

#include "lua_interface.h"
#include "lato.h"

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;
#define COLOR_BLACK (Clay_Color) {0, 0, 0, 255}
#define COLOR_GRAY (Clay_Color) {30, 30, 30, 255}
#define COLOR_LIGHT_GRAY (Clay_Color) {50, 50, 50, 255}
#define COLOR_DARK_BLUE (Clay_Color) {50, 50, 80, 255}
#define COLOR_WHITE (Clay_Color) {255, 255, 255, 255}
#define COLOR_ORANGE (Clay_Color) {225, 138, 50, 255}
#define COLOR_BLUE (Clay_Color) {111, 173, 162, 255}

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) (Clay_Vector2) { .x = vector.x, .y = vector.y }

#define CLAY_DYNSTR(s) ((Clay_String) { .chars = s, .length = strlen(s) })

Clay_TextElementConfig headerTextConfig = {
    .fontId = 1,
    .letterSpacing = 1,
    .fontSize = 16,
    .textColor = COLOR_WHITE
};

int selectedScenarioIndex = -1;
void handleSelectScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selectedScenarioIndex = (int) userData;
    }
}

void RenderScenarioCard(int index, Clay_String path) {
    CLAY({
        .id = CLAY_IDI("ScenarioCard", index),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
            .padding = { 5, 5, 5, 5 },
        },
        .backgroundColor = Clay_Hovered()
            ? COLOR_DARK_BLUE
            : COLOR_LIGHT_GRAY,
    }) {
        Clay_OnHover(handleSelectScenario, index);
        CLAY_TEXT(path, CLAY_TEXT_CONFIG(headerTextConfig));
    }
}

struct ScenarioMetadata {
    char *path;
    char *name;
    char *author;
    char *description;
};
typedef struct ScenarioMetadata ScenarioMetadata;

cvector_vector_type(ScenarioMetadata) fileMetadata = NULL;

void findScenarios() {
    selectedScenarioIndex = -1;
    for (size_t i = 0; i < cvector_size(fileMetadata); i++) {
        free(fileMetadata[i].path);
    }
    cvector_clear(fileMetadata);

    tinydir_dir dir;
    // TODO: don't hardcode
    tinydir_open(&dir, "../../scenarios");

    // TODO: lazy loading
    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (!file.is_dir) {
            continue;
        }
        
        tinydir_dir fileAsDir;
        tinydir_open(&fileAsDir, file.path);

        int check = 0;
        char *tomlFilePath = NULL;
        char *luaFilePath = NULL;
        bool fail = false;
        while (fileAsDir.has_next) {
            tinydir_file scenarioFile;
            tinydir_readfile(&fileAsDir, &scenarioFile);
            if (strcmp(scenarioFile.name, "info.toml") == 0) {
                tomlFilePath = malloc(strlen(scenarioFile.path));
                strcpy(tomlFilePath, scenarioFile.path);
                check++;
            } else if (strcmp(scenarioFile.name, "script.lua") == 0) {
                luaFilePath = malloc(strlen(scenarioFile.path));
                strcpy(luaFilePath, scenarioFile.path);
                check++;
            }
            tinydir_next(&fileAsDir);
        }
        tinydir_close(&fileAsDir);

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

        ScenarioMetadata metadata = {
            .path = luaFilePath,
            .name = name.u.s,
            .author = author.u.s,
            .description = description.u.s,
        };

        cvector_push_back(fileMetadata, metadata);

cleanup:
        if (tomlFilePath != NULL) {
            free(tomlFilePath);
        }
        if (fail && (luaFilePath != NULL)) {
            free(luaFilePath);
        }
        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}

void handleReloadScenarios(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        findScenarios();
    }
}

void handleStartScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        loadLuaScenario(fileMetadata[selectedScenarioIndex].path);
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
                Clay_String path = {
                    .chars = fileMetadata[i].name,
                    .length = strlen(fileMetadata[i].name),
                };
                RenderScenarioCard(i, path);
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
                CLAY_TEXT(CLAY_STRING("Reload"), &headerTextConfig);
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
                CLAY_TEXT(CLAY_STRING("No scenario selected"), CLAY_TEXT_CONFIG(headerTextConfig));
            } else {
                ScenarioMetadata s = fileMetadata[selectedScenarioIndex];
                // TODO: populate
                CLAY_TEXT(
                    CLAY_DYNSTR(s.name),
                    CLAY_TEXT_CONFIG(headerTextConfig)
                );
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                CLAY_TEXT(CLAY_STRING("Author: "), CLAY_TEXT_CONFIG(headerTextConfig));
                CLAY_TEXT(CLAY_DYNSTR(s.author), CLAY_TEXT_CONFIG(headerTextConfig));
                }
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                CLAY_TEXT(CLAY_STRING("Description: "), CLAY_TEXT_CONFIG(headerTextConfig));
                CLAY_TEXT(CLAY_DYNSTR(s.description), CLAY_TEXT_CONFIG(headerTextConfig));
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
                    CLAY_TEXT(CLAY_STRING("Start Scenario"), &headerTextConfig);
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
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() });
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
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(clayMemory, (Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() }, (Clay_ErrorHandler) { HandleClayErrors, 0 });
    Clay_Raylib_Initialize(GetScreenWidth(), GetScreenHeight(), "Clay - Raylib Renderer Example", FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    Font fonts[2];
    fonts[FONT_ID_BODY_24] = LoadFontFromMemory(".ttf", LatoRegularTTF, sizeof(LatoRegularTTF), 24, NULL, 0);
	SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_16] = LoadFontFromMemory(".ttf", LatoRegularTTF, sizeof(LatoRegularTTF), 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    findScenarios();

    SetExitKey(KEY_NULL);
    while (!WindowShouldClose()) {
        if (reinitializeClay) {
            Clay_SetMaxElementCount(8192);
            totalMemorySize = Clay_MinMemorySize();
            clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
            Clay_Initialize(clayMemory, (Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() }, (Clay_ErrorHandler) { HandleClayErrors, 0 });
            reinitializeClay = false;
        }
        UpdateDrawFrame(fonts);
    }

    // Clay_Raylib_Close();
    return 0;
}

#endif /* UI_H */
