#ifndef SETTINGS_COMMON_H
#define SETTINGS_COMMON_H

float sensitivity = 0.5;

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

CrosshairConfig currentCrosshairConfig = DEFAULT_CROSSHAIR_CONFIG;

void drawCrosshair(RenderTexture2D texture, Color clearColor) {
    BeginTextureMode(texture);

    int cw = texture.texture.width / 2;
    int ch = texture.texture.height / 2;

    DrawRectangle(
        0,
        0,
        texture.texture.width,
        texture.texture.height,
        clearColor 
    );

    if (currentCrosshairConfig.centerDot) {
        DrawCircle(
            cw,
            ch,
            currentCrosshairConfig.centerDotRadius,
            WHITE
        );
    }

    if (currentCrosshairConfig.innerLines) {
        DrawRectangle(
            cw + currentCrosshairConfig.innerLineGap,
            ch - currentCrosshairConfig.innerLineWidth / 2.0f,
            currentCrosshairConfig.innerLineLength,
            currentCrosshairConfig.innerLineWidth,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.innerLineGap - currentCrosshairConfig.innerLineLength,
            ch - currentCrosshairConfig.innerLineWidth / 2.0f,
            currentCrosshairConfig.innerLineLength,
            currentCrosshairConfig.innerLineWidth,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.innerLineWidth / 2.0f,
            ch - currentCrosshairConfig.innerLineGap -currentCrosshairConfig.innerLineLength,
            currentCrosshairConfig.innerLineWidth,
            currentCrosshairConfig.innerLineLength,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.innerLineWidth / 2.0f,
            ch + currentCrosshairConfig.innerLineGap,
            currentCrosshairConfig.innerLineWidth,
            currentCrosshairConfig.innerLineLength,
            WHITE
        );
    }

    if (currentCrosshairConfig.outerLines) {
        float innerOffset = currentCrosshairConfig.innerLineGap + currentCrosshairConfig.innerLineLength;
        DrawRectangle(
            cw + currentCrosshairConfig.outerLineGap + innerOffset,
            ch - currentCrosshairConfig.outerLineWidth / 2.0f,
            currentCrosshairConfig.outerLineLength,
            currentCrosshairConfig.outerLineWidth,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.outerLineGap - currentCrosshairConfig.outerLineLength - innerOffset,
            ch - currentCrosshairConfig.outerLineWidth / 2.0f,
            currentCrosshairConfig.outerLineLength,
            currentCrosshairConfig.outerLineWidth,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.outerLineWidth / 2.0f,
            ch - currentCrosshairConfig.outerLineGap -currentCrosshairConfig.outerLineLength - innerOffset,
            currentCrosshairConfig.outerLineWidth,
            currentCrosshairConfig.outerLineLength,
            WHITE
        );
        DrawRectangle(
            cw - currentCrosshairConfig.outerLineWidth / 2.0f,
            ch + currentCrosshairConfig.outerLineGap + innerOffset,
            currentCrosshairConfig.outerLineWidth,
            currentCrosshairConfig.outerLineLength,
            WHITE
        );
    }

    EndTextureMode();
}

#endif /* SETTINGS_COMMON_H */
