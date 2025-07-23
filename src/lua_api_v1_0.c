#include "lua_api_v1_0.h"

#include <float.h>

int v1_0_loadConfig(lua_State *L, v1_0_Config *v1_0_config) {
    lua_getglobal(L, "config");
    
    if (!lua_istable(L, -1)) {
        fprintf(stderr, "`config` table not found\n");
        lua_pop(L, 1);
        return -1;
    }
    
    lua_getfield(L, -1, "piercing");
    v1_0_config->piercing = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "move");
    v1_0_config->move = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "moveBoundingBox");
    if (lua_isnil(L, -1)) {
        v1_0_config->moveBoundingBox = (Rectangle) {
            .x = -100000.0f,
            .y = -100000.0f,
            .width = 200000.0f,
            .height = 200000.0f,
        };
    } else {
        int cx, cy, w, h;
        lua_rawgeti(L, -1, 1);
        cx = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        cy = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        w = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 4);
        h = lua_tonumber(L, -1);
        lua_pop(L, 1);

        v1_0_config->moveBoundingBox.x = cx - w / 2;
        v1_0_config->moveBoundingBox.y = cy - h / 2;
        v1_0_config->moveBoundingBox.width = w;
        v1_0_config->moveBoundingBox.height = h;
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "initialPosition");
        lua_rawgeti(L, -1, 1);
        v1_0_config->initialPosition.x = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        v1_0_config->initialPosition.y = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        v1_0_config->initialPosition.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "initialTarget");
        lua_rawgeti(L, -1, 1);
        v1_0_config->initialTarget.x = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        v1_0_config->initialTarget.y = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 3);
        v1_0_config->initialTarget.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "automatic");
    v1_0_config->automatic = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "shotDelay");
    v1_0_config->shotDelay = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return 0;
}

int v1_0_addSphere(lua_State *L) {
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

int v1_0_drawCuboid(lua_State *L) {
    float x = (float) luaL_checknumber(L, 1);
    float y = (float) luaL_checknumber(L, 2);
    float z = (float) luaL_checknumber(L, 3);
    float w = (float) luaL_checknumber(L, 4);
    float h = (float) luaL_checknumber(L, 5);
    float l = (float) luaL_checknumber(L, 6);

    Mesh cube = GenMeshCube(w, h, l);
    // for (int i = 0; i < cube.vertexCount * 2; i++) {
    //     cube.texcoords[i] *= 2.0f; // Repeat 2x on both axes
    // }

    Model m = LoadModelFromMesh(cube);
    // m.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wallTexture;
    m.materials[0].shader = wallShader;

    cvector_push_back(
        environmentCuboids,
        ((EnvironmentCuboidData) {
            .position = (Vector3) { x, y, z },
            .width = w,
            .height = h,
            .length = l,
            .model = m,
        })
    );

    return 1;
}

int v1_0_getPosition(lua_State *L) {
    long long id = luaL_checknumber(L, 1);
    SMItem *item = sm_get_item(&targetMap, id);
    lua_pushnumber(L, item->value.position.x);
    lua_pushnumber(L, item->value.position.y);
    lua_pushnumber(L, item->value.position.z);
    return 3;
}

int v1_0_setPosition(lua_State *L) {
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

int v1_0_removeTarget(lua_State *L) {
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

int v1_0_incrementHitCount(lua_State *L) {
    hitCount++;
    return 0;
}

int v1_0_incrementShotCount(lua_State *L) {
    shotCount++;
    return 0;
}

int v1_0_getScore(lua_State *L) {
    lua_pushnumber(L, score);
    return 1;
}

int v1_0_setScore(lua_State *L) {
    int newScore = luaL_checknumber(L, 1);
    score = newScore;
    return 0;
}

int v1_0_getTime(lua_State *L) {
    lua_pushnumber(L, elapsedTime);
    return 1;
}

int v1_0_getShotCooldown(lua_State *L) {
    lua_pushnumber(L, shotCooldown);
    return 1;
}

int v1_0_setShotCooldown(lua_State *L) {
    float newShotCooldown = luaL_checknumber(L, 1);
    shotCooldown = newShotCooldown;
    return 0;
}

int v1_0_addUserInfo(lua_State *L) {
    size_t keySize, valueSize;
    const char *key = lua_tolstring(L, 1, &keySize);
    const char *value = lua_tolstring(L, 2, &valueSize);
    char *keyCopy = malloc(keySize + 1);
    strcpy(keyCopy, key);
    char *valueCopy = malloc(valueSize + 1);
    strcpy(valueCopy, value);
    ScenarioUserInfo newUserInfo = {
        .key = keyCopy,
        .value = valueCopy,
    };
    cvector_push_back(scenarioUserInfoList, newUserInfo);
    return 0;
}
