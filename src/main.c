#include "raylib/raylib.h"

#include "ui.h"

int main(void) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    spawnUi();

    return 0;
}

