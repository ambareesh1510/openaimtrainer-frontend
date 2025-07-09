#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#include "slotmap/slotmap.h"
#include "cvector/cvector.h"

#include "target.h"

Slotmap targetMap;
cvector_vector_type(sm_item_id) targetIds = NULL;

int shotCount = 0;
int hitCount = 0;

double elapsedTime = 0.0;

struct {
    Vector3 initialPosition;
    bool piercing;
    bool move;
    double timer;
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

    lua_getfield(L, -1, "timer");
    config.timer = lua_tonumber(L, -1);
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
    cvector_push_back(targetIds, item->id);
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
        if (targetIds[i] == id) {
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

void initLua(lua_State *L) {
    lua_register(L, "addSphere", addSphere);

    lua_register(L, "getPosition", getPosition);
    lua_register(L, "setPosition", setPosition);

    lua_register(L, "removeTarget", removeTarget);

    lua_register(L, "incrementHitCount", incrementHitCount);
    lua_register(L, "incrementShotCount", incrementShotCount);
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

void loadLuaScenario(char *path) {
    DisableCursor();
    Camera camera = { 0 };
    camera.position = (Vector3) { 0.0f, 2.0f, 8.0f };
    camera.target = (Vector3) { 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    targetMap = sm_new();

    shotCount = 0;
    hitCount = 0;
    elapsedTime = 0.0;

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

    if (callLuaFunction(L, "init") != 0) {
        goto cleanup;
    }

    while (!IsKeyPressed(KEY_Q)) {
        elapsedTime += GetFrameTime();
        if (elapsedTime >= config.timer) {
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

        Vector3 move = { 0 };
        if (config.move) {
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

        RayCollision collision;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (callLuaFunction(L, "onShoot") != 0) {
                goto cleanup;
            }
            for (size_t i = 0; i < cvector_size(targetIds); i++) {
                sm_item_id id = targetIds[i];
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

        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(camera);
                DrawPlane(
                    (Vector3) { 0.0f, 0.0f, 0.0f },
                    (Vector2) { 32.0f, 32.0f },
                    GRAY
                );
                
                for (size_t i = 0; i < cvector_size(targetIds); i++) {
                    sm_item_id id = targetIds[i];
                    Target t = sm_get_item(&targetMap, id)->value;
                    switch (t.type) {
                        case SPHERE:
                            DrawSphere(
                                t.position,
                                t.data.sphere.radius,
                                GREEN
                            );
                            break;
                        default:
                            printf("Found invalid target type!");
                            break;
                    }
                }


            EndMode3D();

            DrawCircle(
                GetScreenWidth() / 2,
                GetScreenHeight() / 2,
                2.0f,
                DARKBLUE
            );

            char *s = malloc(100);

            sprintf(s, "%d/%d", hitCount, shotCount);
            DrawText(s, 100, 100, 52, BLACK);

            sprintf(s, "%f", config.timer - elapsedTime);
            DrawText(s, 100, 200, 52, BLACK);

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
