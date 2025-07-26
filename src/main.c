#include "raylib/raylib.h"

#include "ui.h"
#include "network.h"

int main(void) {
    initCurl();
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    spawnUi();

    return 0;
}

