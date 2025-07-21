#ifndef CLAY_RENDERER_RAYLIB_H
#define CLAY_RENDERER_RAYLIB_H

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "cvector/cvector.h"
#include "clay/clay.h"

#include "settings_common.h"

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) (Color) { .r = (unsigned char)roundf(color.r), .g = (unsigned char)roundf(color.g), .b = (unsigned char)roundf(color.b), .a = (unsigned char)roundf(color.a) }

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern RenderTexture2D settingsCrosshairTexture;
extern bool settingsCrosshairTextureInitialized;

struct Clay_ScissorData {
    int x;
    int y;
    int width;
    int height;
    uint32_t id;
};
typedef struct Clay_ScissorData Clay_ScissorData;

Clay_ScissorData clipScissorData(Clay_ScissorData outer, Clay_ScissorData inner);

extern cvector_vector_type(Clay_ScissorData) scissorDataStack;

extern Camera Raylib_camera;

typedef enum
{
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
} CustomLayoutElementType;

typedef struct
{
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
} CustomLayoutElement_3DModel;

typedef struct
{
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    } customData;
} CustomLayoutElement;

// Get a ray trace from the screen position (i.e mouse) within a specific section of the screen
Ray GetScreenToWorldPointWithZDistance(Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance);

static inline Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    // Measure string size for Font
    Clay_Dimensions textSize = { 0 };

    float maxTextWidth = 0.0f;
    float lineTextWidth = 0;
    int maxLineCharCount = 0;
    int lineCharCount = 0;

    float textHeight = config->fontSize;
    Font* fonts = (Font*)userData;
    Font fontToUse = fonts[config->fontId];
    // Font failed to load, likely the fonts are in the wrong place relative to the execution dir.
    // RayLib ships with a default font, so we can continue with that built in one. 
    if (!fontToUse.glyphs) {
        fontToUse = GetFontDefault();
    }

    float scaleFactor = config->fontSize/(float)fontToUse.baseSize;

    for (int i = 0; i < text.length; ++i, lineCharCount++)
    {
        if (text.chars[i] == '\n') {
            maxTextWidth = fmax(maxTextWidth, lineTextWidth);
            maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);
            lineTextWidth = 0;
            lineCharCount = 0;
            continue;
        }
        int index = text.chars[i] - 32;
        if (fontToUse.glyphs[index].advanceX != 0) lineTextWidth += fontToUse.glyphs[index].advanceX;
        else lineTextWidth += (fontToUse.recs[index].width + fontToUse.glyphs[index].offsetX);
    }

    maxTextWidth = fmax(maxTextWidth, lineTextWidth);
    maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);

    textSize.width = maxTextWidth * scaleFactor + (lineCharCount * config->letterSpacing);
    textSize.height = textHeight;

    return textSize;
}

void Clay_Raylib_Initialize(int width, int height, const char *title, unsigned int flags);

// A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free Calls.
// Call Clay_Raylib_Close() to free
static char *temp_render_buffer;
static int temp_render_buffer_len;

// Call after closing the window to clean up the render buffer
void Clay_Raylib_Close();

void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts);

#endif /* CLAY_RENDERER_RAYLIB_H */
