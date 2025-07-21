#ifndef UI_H
#define UI_H

#include "cvector/cvector.h"
#include "tomlc17/tomlc17.h"

#include "clay/clay.h"
#include "clay_renderer_raylib.h"

#include "ui_utils.h"
#include "main_menu.h"
#include "settings.h"
#include "scenario_select.h"
#include "post_scenario.h"
#include "lua_interface.h"
#include "lato.h"


void UpdateDrawFrame(Font* fonts);

void HandleClayErrors(Clay_ErrorData errorData);

int spawnUi(void);

#endif /* UI_H */
