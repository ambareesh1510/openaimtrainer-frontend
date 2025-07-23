#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <stdbool.h>

#include "lato/lato.h"
#include "settings_common.h"
#include "clay_renderer_raylib.h"

enum UiState {
    MAIN_MENU,
    SCENARIO_SELECT,
    SETTINGS,
    POST_SCENARIO,
};
typedef enum UiState UiState;
extern UiState uiState;

struct ScenarioResults {
    bool valid;
    int score;
    float accuracy;
};
typedef struct ScenarioResults ScenarioResults;
extern ScenarioResults scenarioResults;

// TODO: move this to a more appropriate place
struct ScenarioDifficultyData {
    char *difficultyName;
    Clay_Color foregroundColor;
    Clay_Color backgroundColor;
};
typedef struct ScenarioDifficultyData ScenarioDifficultyData;

struct LuaApiVersion {
    int major;
    int minor;
};
typedef struct LuaApiVersion LuaApiVersion;

struct ScenarioMetadata {
    char *path;
    const char *name;
    const char *author;
    const char *description;
    double time;
    double titleOffset;
    double authorOffset;
    double timeOffset;
    ScenarioDifficultyData *difficultyData;
    int numDifficulties;
    LuaApiVersion apiVersion;
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

#define CDIV(w, h) CLAY({ .layout = { .sizing = { .width = (w), .height = (h), }, }, })

#define HCENTER CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), }, .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, }, })
#define VCENTER CLAY({ .layout = { .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }, }, })
#define HVCENTER CLAY({ .layout = { .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, }, })

enum {
    FONT_ID_LARGE_BOLD,
    FONT_ID_BODY_16,
    FONT_ID_BODY_BOLD,
    FONT_ID_HUGE_BOLD,
    FONT_ID_TITLE_BOLD,
    FONT_ID_GRAPH,
    NUM_FONTS,
};

extern Font fonts[NUM_FONTS];

void initFonts();

extern Clay_TextElementConfig largeTextConfig;
extern Clay_TextElementConfig normalTextConfig;
extern Clay_TextElementConfig boldTextConfig;
extern Clay_TextElementConfig hugeTextConfig;
extern Clay_TextElementConfig titleTextConfig;


void hSpacer();

void handleClickCheckbox(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

void renderCheckbox(bool *checked);

// TODO: change progress from (float *) to float
// TODO: remove ID field from this struct
struct SliderData {
    float *progress;
    int id;
    bool held;
    float min;
    float max;
    float snap;
};
typedef struct SliderData SliderData;

void updateSlider(Clay_ElementId elementId, Clay_Vector2 pointerPosition, SliderData *data);

extern bool sliderSelected;
void handleMoveSlider(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

void renderSlider(SliderData *data, float controlWidth);


void handleToScenarioSelect(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleToSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleToMainMenu(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

#endif /* UI_UTILS_H */
