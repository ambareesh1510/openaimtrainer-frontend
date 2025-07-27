#include "scenario_select.h"

#include "tomlc17/tomlc17.h"
#include "fuzzy_match/fuzzy_match.h"

#include "save_scores.h"
#include "network.h"

int selectedScenarioIndex = -1;
int selectedDifficulty = -1;

#define SEARCH_PLACEHOLDER "Search for scenarios..."
TextBoxData scenarioSearchData = {
    .placeholder = SEARCH_PLACEHOLDER,
    .placeholderLen = sizeof(SEARCH_PLACEHOLDER) - 1,
    .id = 1,
};

enum {
    MY_SCENARIOS,
    DOWNLOADED_SCENARIOS,
    ONLINE_SCENARIOS,
} currentScenarioTab;

cvector_vector_type(ScenarioMetadata) myFileMetadata = NULL;
cvector_vector_type(ScenarioMetadata) downloadedFileMetadata = NULL;
cvector_vector_type(ScenarioMetadata) onlineFileMetadata = NULL;

int compareFuzzyScore(const void *a, const void *b) {
    ScenarioMetadata *metadataA = (ScenarioMetadata *) a;
    ScenarioMetadata *metadataB = (ScenarioMetadata *) b;
    int32_t scoreA = fuzzy_match(scenarioSearchData.str, metadataA->name);
    int32_t scoreB = fuzzy_match(scenarioSearchData.str, metadataB->name);
    // printf("%d\n", scoreA - scoreB);
    return scoreA - scoreB;
}

void fuzzySortCurrentMetadataList(void) {
    if (currentScenarioTab == ONLINE_SCENARIOS) {
        return;
    } else if (currentScenarioTab == MY_SCENARIOS) {
        qsort(
            myFileMetadata,
            cvector_size(myFileMetadata),
            sizeof(ScenarioMetadata),
            compareFuzzyScore
        );
    } else if (currentScenarioTab == DOWNLOADED_SCENARIOS) {
        qsort(
            downloadedFileMetadata,
            cvector_size(downloadedFileMetadata),
            sizeof(ScenarioMetadata),
            compareFuzzyScore
        );
    }
}

void handleSelectScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selectedScenarioIndex = (int) userData;
        selectedDifficulty = 0;
        loadSavedScores(myFileMetadata[selectedScenarioIndex]);
    }
}

void handleSelectDifficulty(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selectedDifficulty = (int) userData;
    }
}

void handleSwitchToScenariosTab(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        currentScenarioTab = userData;
    }
}


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
        ScenarioMetadata *metadata;

        if (currentScenarioTab == MY_SCENARIOS) {
            metadata = cvector_at(myFileMetadata, index);
        } else if (currentScenarioTab == DOWNLOADED_SCENARIOS) {
            metadata = cvector_at(downloadedFileMetadata, index);
        } else if (currentScenarioTab == ONLINE_SCENARIOS) {
            metadata = cvector_at(onlineFileMetadata, index);
        }
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
            CLAY_TEXT(CLAY_DYNSTR(metadata->name), CLAY_TEXT_CONFIG(largeTextConfig));
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

int getHexDigit(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return 10 + c - 'a';
    } else {
        return -1;
    }
}

Clay_Color convertHexBufferToColor(const char buf[6]) {
    int rgb[3];
    for (size_t i = 0; i <= 4; i += 2) {
        char b1 = buf[i], b2 = buf[i + 1];
        int v1 = getHexDigit(b1), v2 = getHexDigit(b2);
        if (v1 == -1 || v2 == -1) {
            break;
        }
        rgb[i / 2] = v1 * 16 + v2;
    }
    Clay_Color color = {
        rgb[0],
        rgb[1],
        rgb[2],
        255
    };
    return color;
}

LuaApiVersion parseVersion(const char *versionString) {
    size_t versionLen = strlen(versionString);
    char *buf = malloc(versionLen + 1);
    strcpy(buf, versionString);
    char *i = buf;
    while (*i != '\0') {
        if (*i == '.') {
            *i == '\0';
            i++;
            break;
        }
        i++;
    }
    LuaApiVersion version;
    errno = 0;
    int major = strtol(buf, NULL, 0);
    if (errno == EINVAL) {
        goto bail;
    }

    int minor = strtol(i, NULL, 0);
    if (errno == EINVAL) {
        goto bail;
    }
    version.major = major;
    version.minor = minor;
bail:
    free(buf);
    return version;
}

void findScenarios() {
    selectedScenarioIndex = -1;
    for (size_t i = 0; i < cvector_size(myFileMetadata); i++) {
        free(myFileMetadata[i].path);
        for (size_t j = 0; j < myFileMetadata[i].numDifficulties; j++) {
            free(myFileMetadata[i].difficultyData[j].difficultyName);
        }
        free(myFileMetadata[i].difficultyData);
    }
    cvector_clear(myFileMetadata);


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
        toml_datum_t difficulties = toml_seek(tomlParseResult.toptab, "difficulties");
        toml_datum_t apiVersion = toml_seek(tomlParseResult.toptab, "api_version");

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

        if (apiVersion.type != TOML_STRING) {
            fprintf(stderr, "Invalid key `api_version`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        ScenarioDifficultyData *difficultyData;

        if (difficulties.type == TOML_UNKNOWN) {
            difficultyData = NULL;
        } else if (difficulties.type != TOML_ARRAY) {
            fprintf(stderr, "Invalid key `difficulties`!");
            fail = true;
            toml_free(tomlParseResult);
            goto cleanup;
        }

        // We know that difficulty data is an array
        int difficultyDataFailureIndex = -1;
        difficultyData = malloc(sizeof(*difficultyData) * difficulties.u.arr.size);
        int numDifficulties = difficulties.u.arr.size;
        for (size_t i = 0; i < numDifficulties; i++) {
            toml_datum_t difficultyElem = difficulties.u.arr.elem[i];
            if (difficultyElem.type != TOML_ARRAY || difficultyElem.u.arr.size != 3) {
                difficultyDataFailureIndex = i;
                break;
            }
            toml_datum_t difficultyName = difficultyElem.u.arr.elem[0];
            toml_datum_t difficultyForegroundColor = difficultyElem.u.arr.elem[1];
            toml_datum_t difficultyBackgroundColor = difficultyElem.u.arr.elem[2];
            if (
                difficultyName.type != TOML_STRING
                || difficultyForegroundColor.type != TOML_STRING
                || difficultyBackgroundColor.type != TOML_STRING
            ) {
                difficultyDataFailureIndex = i;
                break;
            }

            Clay_Color fg = convertHexBufferToColor(difficultyForegroundColor.u.s);
            Clay_Color bg = convertHexBufferToColor(difficultyBackgroundColor.u.s);
            char *difficultyNameBuffer = malloc(strlen(difficultyName.u.s) + 1);
            strcpy(difficultyNameBuffer, difficultyName.u.s);
            difficultyData[i] = (ScenarioDifficultyData) {
                .difficultyName = difficultyNameBuffer,
                .foregroundColor = fg,
                .backgroundColor = bg,
            };
        }

        // TODO: test this failure condition
        if (difficultyDataFailureIndex >= 0) {
            for (int i = 0; i < difficultyDataFailureIndex; i++) {
                free(difficultyData[i].difficultyName);
            }
            free(difficultyData);
            difficultyData = NULL;
        }

        if (difficultyData == NULL) {
            numDifficulties = 0;
        }

        LuaApiVersion apiVersion_ = parseVersion(apiVersion.u.s);

        ScenarioMetadata metadata = {
            .path = luaFilePath,
            .name = name.u.s,
            .author = author.u.s,
            .description = description.u.s,
            .time = time.u.fp64,
            .difficultyData = difficultyData,
            .numDifficulties = numDifficulties,
            .apiVersion = apiVersion_,
        };

        cvector_push_back(myFileMetadata, metadata);

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
        char *difficultyName = NULL;
        if (myFileMetadata[selectedScenarioIndex].numDifficulties != 0) {
            difficultyName = myFileMetadata[selectedScenarioIndex].difficultyData[selectedDifficulty].difficultyName;
        }
        loadLuaScenario(
            myFileMetadata[selectedScenarioIndex],
            selectedDifficulty,
            difficultyName
        );
        if (scenarioResults.valid) {
            loadSavedScores(myFileMetadata[selectedScenarioIndex]);
            uiState = POST_SCENARIO;
        } else {
            uiState = SCENARIO_SELECT;
        }
    }
}

SubmitScenarioInfo submitScenarioInfo;

void handlePublishScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        // TODO: remove this check? this should never happen
        if (selectedScenarioIndex < 0 || selectedScenarioIndex > cvector_size(myFileMetadata)) {
            return;
        }

        cleanupSubmitScenarioInfo(&submitScenarioInfo);

        ScenarioMetadata metadata = myFileMetadata[selectedScenarioIndex];
        submitScenarioInfo = createSubmitScenarioInfo(
            metadata.name,
            metadata.author,
            metadata.time,
            TextFormat("%s/info.toml", GetDirectoryPath(metadata.path)),
            metadata.path
        );
        submitScenario(&submitScenarioInfo);
    }
}

// TODO: refer to comment in post_scenario.c
CustomLayoutElementData scenarioSelectScoreGraphData = {
    .type = DRAW_PROGRESSION_GRAPH,
};

void renderScenarioSelectScreen(void) {
    if (IsKeyPressed(KEY_ENTER) && scenarioSearchData.focused) {
        // printf("Sorting metadata list\n");
        fuzzySortCurrentMetadataList();
    }
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            // .padding = { 16, 16, 16, 16 },
            // .childGap = 16
        },
        .backgroundColor = COLOR_GRAY,
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
            .border = {
                .width = { 0, 2, 0, 0 },
                .color = COLOR_LIGHT_GRAY,
            },
        }) {
            CLAY({
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(1.0),
                    },
                },
            }) {

                Clay_ElementDeclaration scenarioTabConfig = {
                    .layout = {
                        .sizing = {
                            // .width = CLAY_SIZING_PERCENT(1.0 / 3.0),
                            .width = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                        .padding = { 5, 5, 5, 5 },
                    }
                };
                scenarioTabConfig.backgroundColor = (currentScenarioTab == MY_SCENARIOS) ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY;
                CLAY(scenarioTabConfig) {
                    Clay_OnHover(handleSwitchToScenariosTab, MY_SCENARIOS);
                    CLAY_TEXT(CLAY_STRING("My Scenarios"), &normalTextConfig);
                }

                scenarioTabConfig.backgroundColor = (currentScenarioTab == DOWNLOADED_SCENARIOS) ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY;
                CLAY(scenarioTabConfig) {
                    Clay_OnHover(handleSwitchToScenariosTab, DOWNLOADED_SCENARIOS);
                    CLAY_TEXT(CLAY_STRING("Downloaded Scenarios"), &normalTextConfig);
                }

                scenarioTabConfig.backgroundColor = (currentScenarioTab == ONLINE_SCENARIOS) ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY;
                CLAY(scenarioTabConfig) {
                    Clay_OnHover(handleSwitchToScenariosTab, ONLINE_SCENARIOS);
                    CLAY_TEXT(CLAY_STRING("Online Scenarios"), &normalTextConfig);
                }
            }
            CDIV(CLAY_SIZING_PERCENT(1.0), CLAY_SIZING_FIXED(50)) {
                renderTextBox(&scenarioSearchData);
            }
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
                    // .childGap = 10,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            }) {
                // TODO: make this list scrollable with arrow keys
                size_t bound = 0;
                if (currentScenarioTab == MY_SCENARIOS) {
                    bound = cvector_size(myFileMetadata);
                } else if (currentScenarioTab == DOWNLOADED_SCENARIOS) {
                    bound = cvector_size(downloadedFileMetadata);
                } else if (currentScenarioTab == ONLINE_SCENARIOS) {
                    bound = cvector_size(onlineFileMetadata);
                }
                for (size_t i = 0; i < bound; i++) {
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
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                            .y = CLAY_ALIGN_Y_CENTER,
                        },
                    },
                }) {
                    CLAY_TEXT(CLAY_STRING("No scenario selected"), CLAY_TEXT_CONFIG(largeTextConfig));
                }
            } else {
                ScenarioMetadata s = myFileMetadata[selectedScenarioIndex];
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
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        // .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childGap = 16,
                    },
                }) {
                    for (size_t i = 0; i < s.numDifficulties; i++) {
                        ScenarioDifficultyData difficultyData = s.difficultyData[i];
                        Clay_BorderElementConfig borderConfig = { 0 };
                        if (i == selectedDifficulty) {
                            borderConfig = (Clay_BorderElementConfig) {
                                .width = { 2, 2, 2, 2 },
                                .color = COLOR_WHITE,
                            };
                        }
                        CLAY({
                            .layout = {
                                .padding = { 5, 5, 5, 5 },
                            },
                            .border = borderConfig,
                            .cornerRadius = { 5, 5, 5, 5 },
                            .backgroundColor = difficultyData.backgroundColor,
                        }) {
                            Clay_OnHover(handleSelectDifficulty, i);
                            Clay_TextElementConfig textConfig = boldTextConfig;
                            textConfig.textColor = difficultyData.foregroundColor;
                            CLAY_TEXT(CLAY_DYNSTR(difficultyData.difficultyName), CLAY_TEXT_CONFIG(textConfig));
                        }
                    }
                }
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_GROW(0),
                        },
                    },
                    .border = {
                        .width = { 2, 2, 2, 2 },
                        .color = COLOR_WHITE,
                    },
                }) {
                    CLAY({
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0),
                            },
                        },
                        .backgroundColor = COLOR_GRAY,
                        .custom = {
                            .customData = &scenarioSelectScoreGraphData,
                        },
                    }) {}
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

                if (authToken != NULL) {
                    CLAY({
                        .backgroundColor = Clay_Hovered()
                            ? COLOR_DARK_BLUE
                            : COLOR_LIGHT_GRAY,
                        .layout = {
                            .padding = { 5, 5, 5, 5 },
                        },
                    }) {
                        Clay_OnHover(handlePublishScenario, 0);
                        CLAY_TEXT(CLAY_STRING("Publish Scenario"), &normalTextConfig);
                    }
                }
            }
        }

    }
}
