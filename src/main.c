#include "raylib/raylib.h"

#include "ui.h"
#include "network.h"
#include "scenario_select.h"
#include "audio.h"
#include "config.h"

int main(void) {
    initDirs();
    initCurl();
    initOnlineFileExtraMetadataMutex();
    initAudio();
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    spawnUi();
    cleanupAudio();

    return 0;
}

