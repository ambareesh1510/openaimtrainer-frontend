#ifndef UI_UTILS_H
#define UI_UTILS_H

#include "lato.h"

// TODO: move this to a more appropriate place
struct ScenarioMetadata {
    char *path;
    const char *name;
    const char *author;
    const char *description;
    double time;
};
typedef struct ScenarioMetadata ScenarioMetadata;

#define COLOR_BLACK (Clay_Color) {0, 0, 0, 255}
#define COLOR_GRAY (Clay_Color) {30, 30, 30, 255}
#define COLOR_LIGHT_GRAY (Clay_Color) {50, 50, 50, 255}
#define COLOR_DARK_BLUE (Clay_Color) {50, 50, 80, 255}
#define COLOR_DARK_GREEN (Clay_Color) {50, 80, 50, 255}
#define COLOR_WHITE (Clay_Color) {255, 255, 255, 255}
#define COLOR_ORANGE (Clay_Color) {225, 138, 50, 255}
#define COLOR_BLUE (Clay_Color) {111, 173, 162, 255}

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) (Clay_Vector2) { .x = vector.x, .y = vector.y }

#define CLAY_DYNSTR(s) ((Clay_String) { .chars = s, .length = strlen(s) })

enum {
    FONT_ID_LARGE_BOLD,
    FONT_ID_BODY_16,
    FONT_ID_BODY_BOLD,
    FONT_ID_HUGE_BOLD,
    NUM_FONTS,
};

Font fonts[NUM_FONTS];

void initFonts() {
    fonts[FONT_ID_LARGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 24, NULL, 0);
	SetTextureFilter(fonts[FONT_ID_LARGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_16] = LoadFontFromMemory(".ttf", LatoRegularTTF, sizeof(LatoRegularTTF), 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 16, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_BODY_BOLD].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_HUGE_BOLD] = LoadFontFromMemory(".ttf", LatoBoldTTF, sizeof(LatoBoldTTF), 36, NULL, 0);
    SetTextureFilter(fonts[FONT_ID_HUGE_BOLD].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
}

Clay_TextElementConfig largeTextConfig = {
    .fontId = FONT_ID_LARGE_BOLD,
    .letterSpacing = 1,
    .fontSize = 24,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig normalTextConfig = {
    .fontId = FONT_ID_BODY_16,
    .letterSpacing = 1,
    .fontSize = 16,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig boldTextConfig = {
    .fontId = FONT_ID_BODY_BOLD,
    .letterSpacing = 1,
    .fontSize = 16,
    .textColor = COLOR_WHITE
};

Clay_TextElementConfig hugeTextConfig = {
    .fontId = FONT_ID_HUGE_BOLD,
    .letterSpacing = 1,
    .fontSize = 36,
    .textColor = COLOR_WHITE
};

void hSpacer() {
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {};
}

#endif /* UI_UTILS_H */
