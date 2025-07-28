#include "raylib/raylib.h"

#include "ui.h"
#include "network.h"
#include "scenario_select.h"

int main(void) {
    initCurl();
    initOnlineFileExtraMetadataMutex();
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    spawnUi();

    return 0;
}

