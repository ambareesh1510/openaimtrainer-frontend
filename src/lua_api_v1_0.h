#ifndef LUA_API_V1_0_H
#define LUA_API_V1_0_H

#include <stdbool.h>

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include "raylib/raylib.h"
#include "slotmap/slotmap.h"
#include "cvector/cvector.h"

#include "lua_interface.h"

struct v1_0_Config {
    Vector3 initialPosition;
    Vector3 initialTarget;
    bool piercing;
    bool move;
    Rectangle moveBoundingBox;
    bool automatic;
    double shotDelay;
};
typedef struct v1_0_Config v1_0_Config;

int v1_0_loadConfig(lua_State *L, v1_0_Config *v1_0_config);
int v1_0_addSphere(lua_State *L);
int v1_0_getPosition(lua_State *L);
int v1_0_setPosition(lua_State *L);
int v1_0_removeTarget(lua_State *L);
int v1_0_incrementHitCount(lua_State *L);
int v1_0_incrementShotCount(lua_State *L);
int v1_0_getScore(lua_State *L);
int v1_0_setScore(lua_State *L);
int v1_0_getTime(lua_State *L);
int v1_0_getShotCooldown(lua_State *L);
int v1_0_setShotCooldown(lua_State *L);
int v1_0_addUserInfo(lua_State *L);

#endif /* LUA_API_V1_0_H */
