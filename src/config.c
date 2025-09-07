#include "config.h"
#include <stdio.h>

char *MY_SCENARIOS_PATH = "";
char *DOWNLOADED_SCENARIOS_PATH = "";

char* ExpandHomeDirectory(const char* path) {
    if (path == NULL || path[0] != '~') {
        return strdup(path); // No expansion needed, return a copy
    }

    const char* home_dir = getenv(HOME_DIR_ENV);
    if (home_dir == NULL) {
        fprintf(stderr, "Failed to get home directory. Using path as-is.");
        return strdup(path); // Fallback to original path
    }

    // Calculate length for the new expanded path string
    size_t home_len = strlen(home_dir);
    size_t path_len = strlen(path);
    size_t new_len = home_len + path_len;

    char* expanded_path = (char*)malloc(new_len);
    if (expanded_path == NULL) {
        fprintf(stderr, "Memory allocation failed for expanding path.");
        return NULL;
    }

    strcpy(expanded_path, home_dir);
    strcat(expanded_path, path + 1); // Concatenate the rest of the path after '~'

    return expanded_path;
}

void initDirs() {
#ifdef _WIN32
#define 
    MY_SCENARIOS_PATH = ExpandHomeDirectory("~/AppData/Roaming/OpenAimTrainer";
    DOWNLOADED_SCENARIOS_PATH = ExpandHomeDirectory("~/AppData/Local/OpenAimTrainer");
#else
    MY_SCENARIOS_PATH = ExpandHomeDirectory("~/.local/share/openaimtrainer");
    DOWNLOADED_SCENARIOS_PATH = ExpandHomeDirectory("~/.cache/openaimtrainer");
#endif
}
