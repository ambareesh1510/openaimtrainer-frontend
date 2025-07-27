#ifndef SCENARIO_SELECT_H
#define SCENARIO_SELECT_H

#include <errno.h>

#include "lua_interface.h"

extern int selectedScenarioIndex;
extern int selectedDifficulty;

void handleSelectScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleSelectDifficulty(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);


extern cvector_vector_type(ScenarioMetadata) myFileMetadata;

// TODO: convert to raylib textformat
extern char *timeBuffer;
void RenderScenarioCard(int index);

int getHexDigit(char c);

Clay_Color convertHexBufferToColor(const char buf[6]);

LuaApiVersion parseVersion(const char *versionString);

void findScenarios();

void handleReloadScenarios(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleStartScenario(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void renderScenarioSelectScreen(void);

#endif /* SCENARIO_SELECT_H */
