#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include <stdbool.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "slotmap/slotmap.h"
#include "cvector/cvector.h"
#include "clay/clay.h"

#include "target.h"
#include "ui_utils.h"
#include "shader.h"
#include "settings_common.h"
#include "lua_api_v1_0.h"

// TODO: consider prefixing lua functions and globals to avoid namespace pollution

extern Slotmap targetMap;

struct TargetData {
    sm_item_id id;
    Model model;
};
typedef struct TargetData TargetData;
extern cvector_vector_type(TargetData) targetIds;

struct EnvironmentCuboidData {
    Vector3 position;
    float width;
    float height;
    float length;
    Model model;
};
typedef struct EnvironmentCuboidData EnvironmentCuboidData;
extern cvector_vector_type(EnvironmentCuboidData) environmentCuboids;

extern int shotCount;
extern int hitCount;
extern int score;

extern double elapsedTime;

extern float shotCooldown;

struct ScenarioConfig {
    Vector3 initialPosition;
    Vector3 initialTarget;
    bool piercing;
    bool move;
    Rectangle moveBoundingBox;
    bool automatic;
    double shotDelay;
};
typedef struct ScenarioConfig ScenarioConfig;

extern ScenarioConfig config;

struct ScenarioUserInfo {
    char *key;
    char *value;
};
typedef struct ScenarioUserInfo ScenarioUserInfo;

extern cvector_vector_type(ScenarioUserInfo) scenarioUserInfoList;

void initLua(lua_State *L);

int callLuaFunction(lua_State *L, char *function);

enum ScenarioState {
    AWAITING_START,
    STARTING,
    STARTED,
};
typedef enum ScenarioState ScenarioState;

extern ScenarioState scenarioState;
// TODO: replace these with raylib textformat
extern char *scenarioTimeBuffer;
extern char *pointsBuffer;
extern char *timerBuffer;
extern char *accuracyBuffer;
extern double countdownToStart;
extern char countdown[2];

Clay_RenderCommandArray scenarioUi(ScenarioMetadata metadata);

extern RenderTexture2D crosshairTexture;
void loadLuaScenario(ScenarioMetadata metadata, int selectedDifficulty, char *selectedDifficultyName);

#endif /* LUA_INTERFACE_H */
