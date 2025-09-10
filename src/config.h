#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <stdlib.h>

#define ASSETS_DIR "assets"

#define SERVER_BASE_URL "https://oats.wtf"
#define API_URL SERVER_BASE_URL "/api"

extern char *MY_SCENARIOS_PATH;
extern char *DOWNLOADED_SCENARIOS_PATH;

#ifdef _WIN32
    #define HOME_DIR_ENV "USERPROFILE"
#else
    #define HOME_DIR_ENV "HOME"
#endif

char* ExpandHomeDirectory(const char* path);

void initDirs();

#endif /* CONFIG_H */
