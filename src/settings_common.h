#ifndef SETTINGS_COMMON_H
#define SETTINGS_COMMON_H

#include <stdbool.h>
#include "raylib/raylib.h"

#define SETTINGS_PATH "./settings"

#define DEFAULT_SENSITIVITY 0.5
extern float sensitivity;

struct CrosshairConfig {
    bool centerDot;
    float centerDotRadius;

    bool innerLines;
    float innerLineWidth;
    float innerLineLength;
    float innerLineGap;

    bool outerLines;
    float outerLineWidth;
    float outerLineLength;
    float outerLineGap;
};
typedef struct CrosshairConfig CrosshairConfig;

#define DEFAULT_CROSSHAIR_CONFIG        \
    {                                   \
        .centerDot = true,              \
        .centerDotRadius = 1.0f,        \
        .innerLines = true,             \
        .innerLineWidth = 2.0f,         \
        .innerLineLength = 5.0f,        \
        .innerLineGap = 5.0f,           \
        .outerLines = true,             \
        .outerLineWidth = 2.0f,         \
        .outerLineLength = 5.0f,        \
        .outerLineGap = 2.0f,           \
    }

extern CrosshairConfig currentCrosshairConfig;

void drawCrosshair(RenderTexture2D texture, Color clearColor);

#define SETTINGS_MAGIC "MAGIC12345"

struct Settings {
    char magic[sizeof(SETTINGS_MAGIC)];
    float sensitivity;
    CrosshairConfig crosshairConfig;
};
typedef struct Settings Settings;

int loadSettings(char *path);

void saveSettings(char *path);

#endif /* SETTINGS_COMMON_H */
