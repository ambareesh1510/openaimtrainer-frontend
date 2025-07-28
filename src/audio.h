#ifndef AUDIO_H
#define AUDIO_H

#include "raylib/raylib.h"

#define MAX_SOUNDS 10
extern Sound soundArray[MAX_SOUNDS];
extern int currentSound;

void initAudio();
void cleanupAudio();

#endif /* AUDIO_H */
