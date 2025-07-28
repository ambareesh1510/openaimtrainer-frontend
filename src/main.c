#include "raylib/raylib.h"

#include "ui.h"
#include "network.h"
#include "scenario_select.h"
#include "audio.h"

int main(void) {
    initCurl();
    initOnlineFileExtraMetadataMutex();
    initAudio();
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    spawnUi();
    cleanupAudio();

    return 0;
}

