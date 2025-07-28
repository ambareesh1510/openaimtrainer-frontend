#include "audio.h"

#include "raylib/raylib.h"
#include "config.h"

Sound soundArray[MAX_SOUNDS] = { 0 };
int currentSound;

void initAudio() {
    InitAudioDevice();

    soundArray[0] = LoadSound(ASSETS_DIR "/shot2.ogg");
    for (int i = 1; i < MAX_SOUNDS; i++) {
        soundArray[i] = LoadSoundAlias(soundArray[0]);
    }
    currentSound = 0;
}

void cleanupAudio() {
    for (int i = 1; i < MAX_SOUNDS; i++) {
        UnloadSoundAlias(soundArray[i]);
    }
    UnloadSound(soundArray[0]);

    CloseAudioDevice();
}
