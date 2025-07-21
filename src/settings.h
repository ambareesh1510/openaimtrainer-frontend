#ifndef SETTINGS_H
#define SETTINGS_H

#include "ui_utils.h"
#include "settings_common.h"

void handleResetCrosshairSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleResetSensitivity(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleExitSettingsMenu(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void handleSaveSettings(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void renderSettingsMenu(void);

#endif /* SETTINGS_H */
