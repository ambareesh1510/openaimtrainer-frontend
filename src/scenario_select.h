#ifndef SCENARIO_SELECT_H
#define SCENARIO_SELECT_H

#include "lua_interface.h"

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
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
            .padding = { 5, 5, 5, 5 },
            .childGap = 16,
        },
        .backgroundColor = (index == selectedScenarioIndex)
            ? COLOR_DARK_GREEN
            : (Clay_Hovered()
                ? COLOR_DARK_BLUE
                : COLOR_LIGHT_GRAY),
    }) {
        Clay_ElementId nameId = CLAY_IDI("ScenarioCard_Name", index);
        Clay_ElementId authorId = CLAY_IDI("ScenarioCard_Author", index);
        Clay_ElementId timeId = CLAY_IDI("ScenarioCard_Time", index);
        ScenarioMetadata *metadata = cvector_at(fileMetadata, index);
        Clay_OnHover(handleSelectScenario, index);
        CLAY({
            .id = nameId,
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.5),
                },
            },
            .clip = {
                .horizontal = true,
                .childOffset = metadata->titleOffset,
            },
        }) {
            if (Clay_Hovered()) {
                float scrollWidth = Clay_GetScrollContainerData(nameId).contentDimensions.width;
                float elementWidth = Clay_GetElementData(nameId).boundingBox.width;
                if (elementWidth < scrollWidth) {
                    metadata->titleOffset -= GetFrameTime() * 60.0 * 1.0;
                    metadata->titleOffset = CLAY__MAX(
                        CLAY__MIN(
                            metadata->titleOffset,
                            0
                        ),
                        -(scrollWidth - elementWidth)
                    );
                }

            } else {
                metadata->titleOffset = 0.0;
            }
            CLAY_TEXT(CLAY_DYNSTR(metadata->name), CLAY_TEXT_CONFIG(boldTextConfig));
        }

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.4),
                },
            },
            .clip = {
                .horizontal = true,
                .childOffset = metadata->authorOffset,
            },
        }) {
            if (Clay_Hovered()) {
                float scrollWidth = Clay_GetScrollContainerData(authorId).contentDimensions.width;
                float elementWidth = Clay_GetElementData(authorId).boundingBox.width;
                if (elementWidth < scrollWidth) {
                    metadata->titleOffset -= 1.0;
                    metadata->titleOffset = CLAY__MAX(
                        CLAY__MIN(
                            metadata->authorOffset,
                            0
                        ),
                        -(scrollWidth - elementWidth)
                    );
                }

            } else {
                metadata->authorOffset = 0.0;
            }
            CLAY_TEXT(CLAY_DYNSTR(metadata->author), CLAY_TEXT_CONFIG(normalTextConfig));
        }

        const char *timeFormat = "%.1fs";
        size_t needed = snprintf(NULL, 0, timeFormat, metadata->time) + 1;
        timeBuffer = realloc(timeBuffer, needed);
        sprintf(timeBuffer, timeFormat, metadata->time);

        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.1),
                },
                .childAlignment = {
                    .x = CLAY_ALIGN_X_RIGHT,
                },
            },
            .clip = {
                .horizontal = true,
                .childOffset = metadata->timeOffset,
            },
        }) {
            if (Clay_Hovered()) {
                float scrollWidth = Clay_GetScrollContainerData(timeId).contentDimensions.width;
                float elementWidth = Clay_GetElementData(timeId).boundingBox.width;
                if (elementWidth < scrollWidth) {
                    metadata->timeOffset -= 1.0;
                    metadata->timeOffset = CLAY__MAX(
                        CLAY__MIN(
                            metadata->timeOffset,
                            0
                        ),
                        -(scrollWidth - elementWidth)
                    );
                }
            } else {
                metadata->timeOffset = 0.0;
            }
            CLAY_TEXT(CLAY_DYNSTR(timeBuffer), CLAY_TEXT_CONFIG(normalTextConfig));
        }
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
        if (scenarioResults.valid) {
            uiState = POST_SCENARIO;
        } else {
            uiState = SCENARIO_SELECT;
        }
    }
}

void renderScenarioSelectScreen(void) {
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
            CLAY({
                .clip = {
                    .vertical = true,
                    .childOffset = Clay_GetScrollOffset(),
                },
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0),
                    },
                    .childGap = 10,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            }) {
                // TODO: make this list scrollable with arrow keys
                for (size_t i = 0; i < cvector_size(fileMetadata); i++) {
                    RenderScenarioCard(i);
                }
            }

            CLAY({
                .layout = {
                    .sizing = {
                        .height = CLAY_SIZING_FIT(0),
                        .width = CLAY_SIZING_GROW(0),
                    },
                    .childAlignment = CLAY_LEFT_TO_RIGHT,
                    .childGap = 16,
                },
                .backgroundColor = COLOR_GRAY,
            }) {
                CLAY({
                    .backgroundColor = Clay_Hovered()
                        ? COLOR_DARK_BLUE
                        : COLOR_LIGHT_GRAY,
                    .layout = {
                        .padding = { 5, 5, 5, 5 },
                    },
                }) {
                    Clay_OnHover(handleToMainMenu, 0);
                    CLAY_TEXT(CLAY_STRING("Back"), &normalTextConfig);
                }

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
}

#endif /* SCENARIO_SELECT_H */
