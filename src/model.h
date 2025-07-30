#ifndef MODEL_H
#define MODEL_H

#include "raylib/raylib.h"

extern Model gunModel;

#define GUN_RECOIL_MAX 0.10f
extern float gunRecoilCorrection;
extern bool gunRecoilIncreasing;

void initModels(void);

#endif /* MODEL_H */
