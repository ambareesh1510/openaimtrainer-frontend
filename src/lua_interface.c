#include "lua_interface.h"
#include "clay_renderer_raylib.h"
#include "save_scores.h"

Slotmap targetMap = { 0 };

cvector_vector_type(TargetData) targetIds = NULL;

cvector_vector_type(EnvironmentCuboidData) environmentCuboids = NULL;

int shotCount = 0;
int hitCount = 0;
int score = 0;

double elapsedTime = 0.0;

float shotCooldown = 0.0f;

ScenarioConfig config = { 0 };

ScenarioState scenarioState = AWAITING_START;

cvector_vector_type(ScenarioUserInfo) scenarioUserInfoList = NULL;

void initLua(lua_State *L) {
    lua_register(L, "addSphere", v1_0_addSphere);

    lua_register(L, "drawCuboid", v1_0_drawCuboid);

    lua_register(L, "getPosition", v1_0_getPosition);
    lua_register(L, "setPosition", v1_0_setPosition);

    lua_register(L, "removeTarget", v1_0_removeTarget);

    lua_register(L, "incrementHitCount", v1_0_incrementHitCount);
    lua_register(L, "incrementShotCount", v1_0_incrementShotCount);

    lua_register(L, "getScore", v1_0_getScore);
    lua_register(L, "setScore", v1_0_setScore);

    lua_register(L, "getTime", v1_0_getTime);

    lua_register(L, "getShotCooldown", v1_0_getShotCooldown);
    lua_register(L, "setShotCooldown", v1_0_setShotCooldown);

    lua_register(L, "addUserInfo", v1_0_addUserInfo);
}

int callLuaFunction(lua_State *L, char *function) {
    lua_getglobal(L, function);

    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "`%s` is not a function\n", function);
        lua_pop(L, 1);
        return -1;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error running `%s`: %s\n", function, lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }
}

char *scenarioTimeBuffer = NULL;
char *pointsBuffer = NULL;
char *timerBuffer = NULL;
char *accuracyBuffer = NULL;
double countdownToStart = 0.0;
char countdown[2] = { 0, 0 };
Clay_RenderCommandArray scenarioUi(ScenarioMetadata metadata) {
    Clay_BeginLayout();
    if (scenarioState == AWAITING_START) {
        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0)
                },
                .childGap = 16,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER,
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = { 0, 0, 0, 200 },
        }) {
            CLAY({
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            }) {
                // TODO: clean this up
                // (can probably reuse the buffer from ui.h)
                const char *scenarioTimeFormat = " (%.1fs)";
                size_t needed = snprintf(NULL, 0, scenarioTimeFormat, metadata.time) + 1;
                scenarioTimeBuffer = realloc(scenarioTimeBuffer, needed);
                sprintf(scenarioTimeBuffer, scenarioTimeFormat, metadata.time);
                CLAY_TEXT(CLAY_DYNSTR(metadata.name), &hugeTextConfig);
                CLAY_TEXT(CLAY_DYNSTR(scenarioTimeBuffer), &hugeTextConfig);
            }
            CLAY_TEXT(CLAY_STRING("Click to start"), &hugeTextConfig);
        }
    } else if (scenarioState == STARTING) {
        // TODO: the wrapper divs in this and the next if block are 
        // essentially identical; bundle them into a component
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
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) {
            countdown[0] = '0' + 1 + (int) countdownToStart;
            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(350),
                        .height = CLAY_SIZING_FIXED(50),
                    },
                    .padding = { 5, 5, 5, 5 },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                        .y = CLAY_ALIGN_Y_CENTER,
                    },
                },
                .backgroundColor = {255, 255, 255, 35},
            }) {
                CLAY_TEXT(CLAY_DYNSTR(countdown), &hugeTextConfig);
            }
        }
    } else if (scenarioState == STARTED) {
        CLAY({
            .id = CLAY_ID("HUDContainer"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) {
            CLAY({
                .id = CLAY_ID("HUDTop"),
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(350),
                        .height = CLAY_SIZING_FIXED(50),
                    },
                    .padding = { 5, 5, 5, 5 },
                },
                .backgroundColor = {255, 255, 255, 35},
            }) {
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(1. / 3.),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                    CLAY_TEXT(
                        CLAY_STRING("Points"),
                        CLAY_TEXT_CONFIG(normalTextConfig)
                    );
                    const char *pointsFormat = "%d";
                    size_t needed = snprintf(NULL, 0, pointsFormat, score) + 1;
                    pointsBuffer = realloc(pointsBuffer, needed);
                    sprintf(pointsBuffer, pointsFormat, score);
                    CLAY_TEXT(
                        CLAY_DYNSTR(pointsBuffer),
                        CLAY_TEXT_CONFIG(largeTextConfig)
                    );
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(1. / 3.),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                    CLAY_TEXT(
                        CLAY_STRING("Time"),
                        CLAY_TEXT_CONFIG(normalTextConfig)
                    );
                    const char *timerFormat = "%.3fs";
                    float timerTime = metadata.time - elapsedTime;
                    size_t needed = snprintf(NULL, 0, timerFormat, timerTime) + 1;
                    timerBuffer = realloc(timerBuffer, needed);
                    sprintf(timerBuffer, timerFormat, timerTime);
                    CLAY_TEXT(
                        CLAY_DYNSTR(timerBuffer),
                        CLAY_TEXT_CONFIG(largeTextConfig)
                    );
                }

                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_PERCENT(1. / 3.),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                }) {
                    CLAY_TEXT(
                        CLAY_STRING("Accuracy"),
                        CLAY_TEXT_CONFIG(normalTextConfig)
                    );
                    const char *accuracyFormat = "%.1f%%";
                    float accuracyPercent = (shotCount != 0)
                        ? (100. * ((float) hitCount) / ((float) shotCount))
                        : (100.);
                    size_t needed = snprintf(NULL, 0, accuracyFormat, accuracyPercent) + 1;
                    accuracyBuffer = realloc(accuracyBuffer, needed);
                    sprintf(accuracyBuffer, accuracyFormat, accuracyPercent);
                    CLAY_TEXT(
                        CLAY_DYNSTR(accuracyBuffer),
                        CLAY_TEXT_CONFIG(largeTextConfig)
                    );
                }
            }
            CLAY({
                .layout = {
                    .sizing = {
                        .height = CLAY_SIZING_FIXED(15),
                        .width = CLAY_SIZING_PERCENT(0.2),
                    },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_LEFT,
                    },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
                .backgroundColor = COLOR_GRAY,
            }) {
                float reloadBarPercent = FloatEquals(config.shotDelay, 0.0f) ? (1.0) : (1.0 - shotCooldown / config.shotDelay);
                CLAY({
                    .layout = {
                        .sizing = {
                            .height = CLAY_SIZING_GROW(0),
                            .width = CLAY_SIZING_PERCENT(reloadBarPercent),
                        },
                    },
                    .backgroundColor = COLOR_BLUE,
                }) {}
            }
            CLAY({
                .layout = {
                    .sizing = {
                        .height = CLAY_SIZING_GROW(0),
                    },
                },
            }) {}
            CLAY({
                .id = CLAY_ID("HUDBottom"),
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_FIT(0),
                    },
                    .padding = { 5, 5, 5, 5 },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_RIGHT,
                    },
                },
            }) {
                for (size_t i = 0; i < cvector_size(scenarioUserInfoList); i++) {
                    CLAY({
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_FIT(150),
                                .height = CLAY_SIZING_GROW(0),
                            },
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_CENTER,
                            },
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .padding = { 5, 5, 5, 5 },
                        },
                        .backgroundColor = {255, 255, 255, 35},
                    }) {
                        CLAY_TEXT(
                            CLAY_DYNSTR(scenarioUserInfoList[i].key),
                            CLAY_TEXT_CONFIG(normalTextConfig)
                        );
                        CLAY_TEXT(
                            CLAY_DYNSTR(scenarioUserInfoList[i].value),
                            CLAY_TEXT_CONFIG(largeTextConfig)
                        );
                    }
                }
            }
        }
    }
    return Clay_EndLayout();
}

RenderTexture2D crosshairTexture = { 0 };

int loadConfig(ScenarioMetadata metadata, lua_State *L) {
    LuaApiVersion version = metadata.apiVersion;
    if (version.major == 1 && version.minor == 0) {
        v1_0_Config c;
        if (v1_0_loadConfig(L, &c) != 0) {
            return -1;
        };
        config.initialPosition = c.initialPosition;
        config.initialTarget = c.initialTarget;
        config.piercing = c.piercing;
        config.move = c.move;
        config.moveBoundingBox = c.moveBoundingBox;
        config.automatic = c.automatic;
        config.shotDelay = c.shotDelay;
    } else {
        fprintf(stderr, "Unrecognized Lua API version!\n");
        return -1;
    }
    return 0;
}

void loadLuaScenario(ScenarioMetadata metadata, int selectedDifficulty, char *selectedDifficultyName) {
    if (loadSettings(SETTINGS_PATH) != 0) {
        fprintf(stderr, "Failed to load settings file upon starting scenario\n");
        currentCrosshairConfig = (CrosshairConfig) DEFAULT_CROSSHAIR_CONFIG;
    }
    bool valid = false;
    char *path = metadata.path;
    DisableCursor();
    initShaders();

    targetMap = sm_new();

    shotCount = 0;
    hitCount = 0;
    score = 0;
    elapsedTime = 0.0;
    shotCooldown = 0.0f;
    scenarioState = AWAITING_START;
    cvector_clear(scoreSamples);
    cvector_push_back(scoreSamples, ((ScoreSample) { .score = 0, .accuracy = 100.0f }));

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    initLua(L);

    if (selectedDifficultyName != NULL) {
        lua_pushnumber(L, selectedDifficulty);
        lua_setglobal(L, "difficultyIndex");
        lua_pushstring(L, selectedDifficultyName);
        lua_setglobal(L, "difficulty");
    }

    lua_pushnumber(L, metadata.time);
    lua_setglobal(L, "totalTime");

    if (luaL_dofile(L, path)) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        goto cleanup;
    }

    if (loadConfig(metadata, L) != 0) {
        goto cleanup;
    }

    Camera camera = { 0 };
    camera.position = config.initialPosition;
    camera.target = config.initialTarget;
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    if (callLuaFunction(L, "init") != 0) {
        goto cleanup;
    }

    crosshairTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    drawCrosshair(crosshairTexture, BLANK);

    // TODO: handle WindowShouldClose() separately (currently only exits
    // the scenario)
    while (!(WindowShouldClose() || IsKeyPressed(KEY_ESCAPE))) {
        for (size_t i = 0; i < cvector_size(scenarioUserInfoList); i++) {
            free(scenarioUserInfoList[i].key);
            free(scenarioUserInfoList[i].value);
        }
        cvector_clear(scenarioUserInfoList);

        // TODO: should this use deltaTime?
        Vector3 move = { 0 };
        if (config.move && scenarioState == STARTED) {
            move = (Vector3) {
                (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 0.1f -
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,    
                (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f -
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
                0.0f
            };
        }
        UpdateCameraPro(&camera,
            move,
            (Vector3) {
                GetMouseDelta().x * sensitivity / 10.0f, 
                GetMouseDelta().y * sensitivity / 10.0f,
                0.0f
            },
            0.0f
        );
        float clampedX = fmin(
            fmax(
                camera.position.x,
                config.moveBoundingBox.x
            ),
            config.moveBoundingBox.x + config.moveBoundingBox.width
        );
        float clampedZ = fmin(
            fmax(
                camera.position.z,
                config.moveBoundingBox.y
            ),
            config.moveBoundingBox.y + config.moveBoundingBox.height
        );

        camera.target.x += (clampedX - camera.position.x);
        camera.target.z += (clampedZ - camera.position.z);
        camera.position.x = clampedX;
        camera.position.z = clampedZ;

        if (scenarioState == AWAITING_START) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                scenarioState = STARTING;
                countdownToStart = 3.0;
            }
        }
        if (scenarioState == STARTING) {
            countdownToStart -= GetFrameTime();
            if (countdownToStart < 0.0) {
                scenarioState = STARTED;
            }
        }
        if (scenarioState == STARTED) {
            elapsedTime += GetFrameTime();
            if ((int) elapsedTime > cvector_size(scoreSamples) - 1) {
                ScoreSample s;
                s.score = score;
                if (shotCount == 0) {
                    s.accuracy = 100.0f;
                } else {
                    s.accuracy = 100.0f * hitCount / shotCount;
                }
                cvector_push_back(scoreSamples, s);
                scenarioScoresModified = true;
            }

            shotCooldown -= GetFrameTime();
            if (shotCooldown < 0.0f) {
                shotCooldown = 0.0f;
            }

            if (elapsedTime >= metadata.time) {
                valid = true;
                break;
            }
            Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
            ray.position = camera.position;
            ray.direction.x = camera.target.x - camera.position.x;
            ray.direction.y = camera.target.y - camera.position.y;
            ray.direction.z = camera.target.z - camera.position.z;
            ray.direction = Vector3Normalize(ray.direction);
            if (callLuaFunction(L, "update") != 0) {
                goto cleanup;
            }

            RayCollision collision;
            bool canShoot = FloatEquals(shotCooldown, 0.0f) && (config.automatic
                ? IsMouseButtonDown(MOUSE_BUTTON_LEFT)
                : IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
            if (canShoot) {
                shotCooldown = config.shotDelay;
                if (callLuaFunction(L, "onShoot") != 0) {
                    goto cleanup;
                }
                for (size_t i = 0; i < cvector_size(targetIds); i++) {
                    sm_item_id id = targetIds[i].id;
                    Target t = sm_get_item(&targetMap, id)->value;
                    switch (t.type) {
                        case SPHERE:
                            collision = GetRayCollisionSphere(
                                ray,
                                t.position,
                                t.data.sphere.radius
                            );
                            if (collision.hit) {
                                lua_getglobal(L, "onHit");

                                lua_pushnumber(L, id);

                                if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                                    fprintf(
                                        stderr,
                                        "Error running `%s`: %s\n",
                                        "onHit",
                                        lua_tostring(L, -1)
                                    );
                                    lua_pop(L, 1);
                                    goto cleanup;
                                }
                                if (!config.piercing) {
                                    goto afterCollisionCheck;
                                }
                            }
                            break;
                        default:
                            printf("Found invalid target type!");
                            break;
                    }
                }
    afterCollisionCheck:
            }
        }
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(wallShader, wallShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(camera);
            BeginShaderMode(shader);
                for (size_t i = 0; i < cvector_size(environmentCuboids); i++) {
                    EnvironmentCuboidData data = environmentCuboids[i];
                    int modelLoc = GetShaderLocation(wallShader, "model");
                    SetShaderValueMatrix(wallShader, modelLoc, data.model.transform);
                    DrawModel(data.model, data.position, 1.0f, WHITE);
                }

                for (size_t i = 0; i < cvector_size(targetIds); i++) {
                    sm_item_id id = targetIds[i].id;
                    Target t = sm_get_item(&targetMap, id)->value;
                    switch (t.type) {
                        case SPHERE:
                            DrawModel(
                                targetIds[i].model,
                                t.position,
                                1.0f,
                                // TODO: remove magic numbers
                                (Color) { 25, 153, 189, 255 }
                            );
                            break;
                        default:
                            printf("Found invalid target type!");
                            break;
                    }
                }

            EndShaderMode();
            EndMode3D();

            if (scenarioState == STARTING || scenarioState == STARTED) {
                DrawTexture(crosshairTexture.texture, 0, 0, WHITE);
            }

            Clay_RenderCommandArray hud = scenarioUi(metadata);
            Clay_Raylib_Render(hud, fonts);

        EndDrawing();
    }


cleanup:
    UnloadRenderTexture(crosshairTexture);
    sm_destroy(&targetMap);
    // TODO: need to unload meshes in targetIds
    cvector_clear(targetIds);
    cvector_shrink_to_fit(targetIds);
    cvector_clear(environmentCuboids);
    lua_close(L);
    EnableCursor();

    scenarioResults.valid = valid;
    scenarioResults.score = score;
    scenarioResults.accuracy =
        (shotCount != 0)
            ? (100. * ((float) hitCount) / ((float) shotCount))
            : (100.);

    if (valid) {
        SavedScore newScore = {
            .magic = SAVED_SCORE_MAGIC,
            .score = scenarioResults.score,
            .accuracy = scenarioResults.accuracy,
        };
        saveScore(metadata, newScore);
    }
}
