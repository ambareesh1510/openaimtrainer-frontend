#include "shader.h"

#include <stdio.h>
#include "raylib/rlgl.h"
#include "config.h"

bool shadersInitialized = false;
Shader shader = { 0 };
Shader wallShader = { 0 };

void initShaders() {
    if (shadersInitialized) {
        return;
    }

    shader = LoadShader("lighting.vert",
                               "lighting.frag");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    
    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

    wallShader = LoadShader(
        ASSETS_DIR "/wall_shader.vert",
        ASSETS_DIR "/wall_shader.frag"
    );
    wallShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(wallShader, "viewPos");
    wallShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(wallShader, "matModel");

    ambientLoc = GetShaderLocation(wallShader, "ambient");
    SetShaderValue(wallShader, ambientLoc, (float[4]){ 0.05f, 0.05f, 0.05f, 1.0f }, SHADER_UNIFORM_VEC4);

    shadersInitialized = true;
}

