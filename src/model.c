#include "model.h"

#include "raylib/raylib.h"
#include "config.h"

Model gunModel = { 0 };
float gunRecoilCorrection = 0.0f;
bool gunRecoilIncreasing = false;

void initModels(void) {
    gunModel = LoadModel(ASSETS_DIR "/Pistol.glb");
}
