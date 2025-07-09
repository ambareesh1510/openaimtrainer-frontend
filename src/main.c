#include <raylib.h>
#include <rcamera.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "slotmap/slotmap.h"

#include "target.h"
#include "ui.h"
#include "lua_interface.h"

int main(void) {
    // const int screenWidth = 1920;
    // const int screenHeight = 1080;
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Aim Trainer");

    // TODO: is this necessary? it is done in ui.h anyways
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    spawnUi();

    CloseWindow();

    return 0;
}

