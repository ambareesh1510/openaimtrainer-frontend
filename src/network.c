#include "network.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tinycthread/tinycthread.h"
#include "cjson/cJSON.h"
#include "raylib/raylib.h"

#include "config.h"
#include "scenario_select.h"

bool curlInitialized = false;

void initCurl(void) {
    if (curlInitialized) {
        return;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curlInitialized = true;
}

char *authToken = NULL;
char *username = NULL;

void extractAuthDetails(StrBuf response, char **tokenBuf, char **usernameBuf) {
    cJSON *json = cJSON_Parse(response.buf);
    if (!json) {
        fprintf(stderr, "Could not parse response as JSON");
        return;
    }

    cJSON *tokenField = cJSON_GetObjectItemCaseSensitive(json, "token");
    if (cJSON_IsString(tokenField) && tokenField->valuestring != NULL) {
        size_t bufSize = strlen(tokenField->valuestring) + 1;
        *tokenBuf = realloc(*tokenBuf, bufSize);
        strcpy(*tokenBuf, tokenField->valuestring);
        (*tokenBuf)[bufSize - 1] = '\0';
    } else {
        fprintf(stderr, "No token found in response\n");
    }

    cJSON *usernameField = cJSON_GetObjectItemCaseSensitive(json, "username");
    if (cJSON_IsString(usernameField) && usernameField->valuestring != NULL) {
        size_t bufSize = strlen(usernameField->valuestring) + 1;
        *usernameBuf = realloc(*usernameBuf, bufSize);
        strcpy(*usernameBuf, usernameField->valuestring);
        (*usernameBuf)[bufSize - 1] = '\0';
    } else {
        fprintf(stderr, "No username found in response\n");
    }
    printf("Signed in as %s with token %s\n", usernameBuf, tokenBuf);

    cJSON_Delete(json);
}

void initRequestData(RequestData *data) {
    mtx_init(&data->mutex, mtx_plain);
    data->finished = false;
    data->dispatched = false;
    if (data->response.buf != NULL) {
        free(data->response.buf);
        data->response.buf = NULL;
    }
    data->response.len = 0;
}

void destroyRequestData(RequestData *data) {
    mtx_destroy(&data->mutex);
}

size_t writeToStrBufCallback(
    char *ptr,
    size_t size,
    size_t nmemb,
    void *userdata
) {
    size_t total = size * nmemb;
    StrBuf *data = (StrBuf *) userdata;

    char *newData = realloc(data->buf, data->len + total + 1);
    if (!newData) {
        fprintf(stderr, "realloc() failed\n");
        return 0;
    }

    memcpy(newData + data->len, ptr, total);
    data->buf = newData;
    data->len += total;
    data->buf[data->len] = '\0';

    return total;
}

// =================
// Sign up (/signup)
// =================

AuthRequestInfo createAuthRequestInfo(
    AuthRequestType type,
    const char *username,
    const char *email,
    const char *password
) {
    AuthRequestInfo info = {
        .type = type,
        .username = username,
        .email = email,
        .password = password,
    };
    initRequestData(&info.requestData);
    return info;
}

void cleanupAuthRequestInfo(AuthRequestInfo *info) {
    destroyRequestData(&info->requestData);
}

int sendAuthRequestThread(void *arg) {
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        return -1;
    }

    AuthRequestInfo *info = (AuthRequestInfo *) arg;

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    if (info->type == AUTH_REQUEST_SIGN_UP) {
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "username");
        curl_mime_data(part, info->username, CURL_ZERO_TERMINATED);
    }

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "email");
    curl_mime_data(part, info->email, CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "password");
    curl_mime_data(part, info->password, CURL_ZERO_TERMINATED);

    if (info->type == AUTH_REQUEST_SIGN_UP) {
        curl_easy_setopt(curl, CURLOPT_URL, API_URL "/signup");
    } else if (info->type == AUTH_REQUEST_LOG_IN) {
        curl_easy_setopt(curl, CURLOPT_URL, API_URL "/login");
    } else {
        fprintf(stderr, "ERROR: UNKNOWN AUTH REQUEST TYPE\n");
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStrBufCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->requestData.response);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    mtx_lock(&info->requestData.mutex);
    info->requestData.finished = false;
    mtx_unlock(&info->requestData.mutex);

    CURLcode res = curl_easy_perform(curl);
    // TODO: set return value properly here
    if (res != CURLE_OK) {
        goto bail;
    }

bail:
    curl_easy_cleanup(curl);
    mtx_lock(&info->requestData.mutex);
    // TODO: this is not thread safe
    extractAuthDetails(info->requestData.response, &authToken, &username);
    info->requestData.finished = true;
    mtx_unlock(&info->requestData.mutex);
    printf("/signup OR /login: %s\n", info->requestData.response.buf);
    return 0;
}

int sendAuthRequest(AuthRequestInfo *info) {
    thrd_t threadId;
    int res = thrd_create(&threadId, sendAuthRequestThread, info);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
    info->requestData.dispatched = true;
    return 0;
}

// =================================
// Submit scenario (/createScenario)
// =================================

SubmitScenarioInfo createSubmitScenarioInfo(
    const char *name,
    const char *author,
    double time,
    const char *infoPath,
    const char *scriptPath
) {
    SubmitScenarioInfo info = {
        .name = name,
        .author = author,
        .time = time,
        .infoPath = infoPath,
        .scriptPath = scriptPath,
    };
    initRequestData(&info.requestData);
    return info;
}

void cleanupSubmitScenarioInfo(SubmitScenarioInfo *info) {
    destroyRequestData(&info->requestData);
}

int submitScenarioThread(void *arg) {
    // TODO: reuse curl handle
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        return -1;
    }

    SubmitScenarioInfo *info = (SubmitScenarioInfo *) arg;

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "name");
    curl_mime_data(part, info->name, CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "author");
    curl_mime_data(part, info->author, CURL_ZERO_TERMINATED);

    char timeStr[64];
    snprintf(timeStr, sizeof(timeStr), "%f", info->time);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "time");
    curl_mime_data(part, timeStr, CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "info.toml");
    curl_mime_filedata(part, info->infoPath);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "script.lua");
    curl_mime_filedata(part, info->scriptPath);

    struct curl_slist *headers = NULL;
    char authHeader[512];
    snprintf(authHeader, sizeof(authHeader), "Authorization: %s", authToken);
    printf("Sending header: %s\n", authHeader);
    headers = curl_slist_append(headers, authHeader);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL "/createScenario");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStrBufCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->requestData.response);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    mtx_lock(&info->requestData.mutex);
    info->requestData.finished = false;
    mtx_unlock(&info->requestData.mutex);

    CURLcode res = curl_easy_perform(curl);
    // TODO: set return value properly here
    if (res != CURLE_OK) {
        goto bail;
    }

bail:
    curl_easy_cleanup(curl);
    mtx_lock(&info->requestData.mutex);
    info->requestData.finished = true;
    mtx_unlock(&info->requestData.mutex);
    printf("/createScenario: %s\n", info->requestData.response.buf);
    return 0;
}

int submitScenario(SubmitScenarioInfo *info) {
    thrd_t threadId;
    int res = thrd_create(&threadId, submitScenarioThread, info);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
    info->requestData.dispatched = true;
    return 0;
}

// ===============================
// Find scenarios (/findScenarios)
// ===============================

FindScenariosInfo createFindScenariosInfo(const char *query) {
    FindScenariosInfo info = {
        .query = query,
    };
    initRequestData(&info.requestData);
    return info;
}
void cleanupFindScenariosInfo(FindScenariosInfo *info) {
    destroyRequestData(&info->requestData);
}

int sendFindScenariosRequestThread(void *arg) {
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        return -1;
    }

    FindScenariosInfo *info = (FindScenariosInfo *) arg;

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "query");
    curl_mime_data(part, info->query, CURL_ZERO_TERMINATED);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL "/findScenarios");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStrBufCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->requestData.response);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    mtx_lock(&info->requestData.mutex);
    info->requestData.finished = false;
    mtx_unlock(&info->requestData.mutex);

    CURLcode res = curl_easy_perform(curl);
    // TODO: set return value properly here
    if (res != CURLE_OK) {
        goto bail;
    }

bail:
    curl_easy_cleanup(curl);
    mtx_lock(&info->requestData.mutex);
    info->requestData.finished = true;
    mtx_unlock(&info->requestData.mutex);
    printf("/findScenarios: %s\n", info->requestData.response.buf);
    return 0;
}

int sendFindScenariosRequest(FindScenariosInfo *info) {
    thrd_t threadId;
    int res = thrd_create(&threadId, sendFindScenariosRequestThread, info);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
    info->requestData.dispatched = true;
    return 0;
}

// =================
// Download scenario
// =================

size_t writeFileCallback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *fp = (FILE *)userdata;
    return fwrite(ptr, size, nmemb, fp);
}

int downloadFile(const char *url, const char *destPath) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    FILE *fp = fopen(destPath, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

int downloadScenarioThread(void *arg) {
    const char *uuid = (char *) arg;
    const char *dirPath = TextFormat("%s/%s", DOWNLOADED_SCENARIOS_PATH, uuid);
    int res = MakeDirectory(dirPath);
    if (res != 0) {
        return -1;
    }

    const char *urlInfo = TextFormat("%s/scenarios/%s/info.toml", SERVER_BASE_URL, uuid);
    const char *urlScript = TextFormat("%s/scenarios/%s/script.lua", SERVER_BASE_URL, uuid);

    const char *destInfo = TextFormat("%s/info.toml", dirPath);
    const char *destScript = TextFormat("%s/script.lua", dirPath);

    if (downloadFile(urlInfo, destInfo) != 0) {
        fprintf(stderr, "Failed to download %s\n", urlInfo);
        return -1;
    }

    if (downloadFile(urlScript, destScript) != 0) {
        fprintf(stderr, "Failed to download %s\n", urlScript);
        return -1;
    }

    updateDownloadedInfo();

    return 0;
}

int downloadScenario(const char *uuid) {
    printf("Got download req\n");
    if (DirectoryExists(
        TextFormat(DOWNLOADED_SCENARIOS_PATH "/%s", uuid)
    )) {
        return 0;
    }
    printf("Staring down req\n");
    thrd_t threadId;
    int res = thrd_create(&threadId, downloadScenarioThread, (void *) uuid);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
    return 0;
}
