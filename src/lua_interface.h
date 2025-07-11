#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#include "slotmap/slotmap.h"
#include "cvector/cvector.h"

#include "target.h"
#include "ui_utils.h"
#include "shader.h"

Slotmap targetMap;

struct TargetData {
    sm_item_id id;
    Model model;
};
typedef struct TargetData TargetData;
cvector_vector_type(TargetData) targetIds = NULL;

int shotCount = 0;
int hitCount = 0;
int score = 0;

double elapsedTime = 0.0;

struct {
    Vector3 initialPosition;
    Vector3 initialTarget;
    bool piercing;
    bool move;
} config;

int loadConfig(lua_State *L) {
    lua_getglobal(L, "config");
    
    if (!lua_istable(L, -1)) {
        fprintf(stderr, "`config` table not found\n");
        lua_pop(L, 1);
        return -1;
    }
    
    lua_getfield(L, -1, "piercing");
    config.piercing = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "move");
    config.move = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "initialPosition");
        lua_rawgeti(L, -1, 1);
        config.initialPosition.x = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        config.initialPosition.y = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        config.initialPosition.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "initialTarget");
        lua_rawgeti(L, -1, 1);
        config.initialTarget.x = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        config.initialTarget.y = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        config.initialTarget.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
    lua_pop(L, 1);

    return 0;
}

int addSphere(lua_State *L) {
    float x = (float) luaL_checknumber(L, 1);
    float y = (float) luaL_checknumber(L, 2);
    float z = (float) luaL_checknumber(L, 3);
    float r = (float) luaL_checknumber(L, 4);

    Sphere sphere = {
        .radius = r,
    };

    Target sphereTarget;
    sphereTarget.position = (Vector3) {
        x, y, z
    },
    sphereTarget.type = SPHERE;
    sphereTarget.data.sphere = sphere;

    SMItem *item = sm_create_item(&targetMap);
    item->value = sphereTarget;

    Model m = LoadModelFromMesh(
        GenMeshSphere(r, 32, 32)
    );
    m.materials[0].shader = shader;
    
    TargetData t = {
        .id = item->id,
        .model = m,
    };
    cvector_push_back(targetIds, t);
    lua_pushnumber(L, item->id);

    return 1;
}

int getPosition(lua_State *L) {
    long long id = luaL_checknumber(L, 1);
    SMItem *item = sm_get_item(&targetMap, id);
    lua_pushnumber(L, item->value.position.x);
    lua_pushnumber(L, item->value.position.y);
    lua_pushnumber(L, item->value.position.z);
    return 3;
}

int setPosition(lua_State *L) {
    long long id = luaL_checknumber(L, 1);
    SMItem *item = sm_get_item(&targetMap, id);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);
    item->value.position = (Vector3) {
        .x = x, .y = y, .z = z
    };
    return 0;
}

int removeTarget(lua_State *L) {
    long long id = luaL_checknumber(L, 1);
    for (size_t i = 0; i < cvector_size(targetIds); i++) {
        if (targetIds[i].id == id) {
            UnloadModel(targetIds[i].model);
            cvector_erase(targetIds, i);
        }
    }
    sm_remove_item(&targetMap, id);
    return 0;
}

int incrementHitCount(lua_State *L) {
    hitCount++;
    return 0;
}

int incrementShotCount(lua_State *L) {
    shotCount++;
    return 0;
}

int getScore(lua_State *L) {
    lua_pushnumber(L, score);
    return 1;
}

int setScore (lua_State *L) {
    int newScore = luaL_checknumber(L, 1);
    score = newScore;
    return 0;
}

void initLua(lua_State *L) {
    lua_register(L, "addSphere", addSphere);

    lua_register(L, "getPosition", getPosition);
    lua_register(L, "setPosition", setPosition);

    lua_register(L, "removeTarget", removeTarget);

    lua_register(L, "incrementHitCount", incrementHitCount);
    lua_register(L, "incrementShotCount", incrementShotCount);

    lua_register(L, "getScore", getScore);
    lua_register(L, "setScore", setScore);
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

enum {
    AWAITING_START,
    STARTING,
    STARTED,
} scenarioState = AWAITING_START;
char *scenarioTimeBuffer = NULL;
char *pointsBuffer = NULL;
char *timerBuffer = NULL;
char *accuracyBuffer = NULL;
double countdownToStart = 3.0;
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
                .childGap = 16
            },
        }) {
            hSpacer();
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
            hSpacer();
        }
    }
    return Clay_EndLayout();
}

void loadLuaScenario(ScenarioMetadata metadata) {
    char *path = metadata.path;
    DisableCursor();
    initShaders();

    targetMap = sm_new();

    shotCount = 0;
    hitCount = 0;
    score = 0;
    elapsedTime = 0.0;
    scenarioState = AWAITING_START;

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    initLua(L);

    if (luaL_dofile(L, path)) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        goto cleanup;
    }

    if (loadConfig(L) != 0) {
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

    // TODO: handle WindowShouldClose() separately (currently only exits
    // the scenario)
    while (!(WindowShouldClose() || IsKeyPressed(KEY_ESCAPE))) {
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
                GetMouseDelta().x * 0.05f,
                GetMouseDelta().y * 0.05f,
                0.0f
            },
            GetMouseWheelMove() * 2.0f
        );


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
            if (elapsedTime >= metadata.time) {
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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

        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(camera);
            BeginShaderMode(shader);
                DrawPlane(
                    (Vector3) { 0.0f, 0.0f, 0.0f },
                    (Vector2) { 32.0f, 32.0f },
                    GRAY
                );

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
                DrawCircle(
                    GetScreenWidth() / 2,
                    GetScreenHeight() / 2,
                    2.0f,
                    WHITE
                );
            }

            Clay_RenderCommandArray hud = scenarioUi(metadata);
            Clay_Raylib_Render(hud, fonts);

        EndDrawing();
    }


cleanup:
    sm_destroy(&targetMap);
    cvector_clear(targetIds);
    cvector_shrink_to_fit(targetIds);
    lua_close(L);
    EnableCursor();
}

#endif /* LUA_INTERFACE_H */
