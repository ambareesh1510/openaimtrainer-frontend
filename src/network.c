#include "network.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tinycthread/tinycthread.h"
#include "cjson/cJSON.h"

#define SERVER_BASE_URL "http://localhost"
#define API_URL SERVER_BASE_URL "/api"

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

void freeStrBuf(StrBuf *data) {
    free(data->buf);
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
    char *username,
    char *email,
    char *password
) {
    AuthRequestInfo info = {
        .type = type,
        .username = username,
        .email = email,
        .password = password,
        .finished = true,
    };
    mtx_init(&info.mutex, mtx_plain);
    return info;
}

void cleanupAuthRequestInfo(AuthRequestInfo *info) {
    if (info->response.buf != NULL) {
        free(info->response.buf);
    }
    mtx_destroy(&info->mutex);
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->response);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    mtx_lock(&info->mutex);
    info->finished = false;
    mtx_unlock(&info->mutex);

    CURLcode res = curl_easy_perform(curl);
    // TODO: set return value properly here
    if (res != CURLE_OK) {
        goto bail;
    }

bail:
    curl_easy_cleanup(curl);
    mtx_lock(&info->mutex);
    extractAuthDetails(info->response, &authToken, &username);
    info->finished = true;
    mtx_unlock(&info->mutex);
    printf("/signup OR /login: %s\n", info->response.buf);
    return 0;
}

int sendAuthRequest(AuthRequestInfo *info) {
    thrd_t threadId;
    int res = thrd_create(&threadId, sendAuthRequestThread, info);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
}

// =================================
// Submit scenario (/createScenario)
// =================================

SubmitScenarioInfo createSubmitScenarioInfo(
    char *name,
    char *author,
    double time,
    char *infoPath,
    char *scriptPath
) {
    SubmitScenarioInfo info = {
        .name = name,
        .author = author,
        .time = time,
        .infoPath = infoPath,
        .scriptPath = scriptPath,
        .finished = true,
    };
    mtx_init(&info.mutex, mtx_plain);
    return info;
}

void cleanupSubmitScenarioInfo(SubmitScenarioInfo *info) {
    if (info->response.buf != NULL) {
        free(info->response.buf);
    }
    mtx_destroy(&info->mutex);
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

    curl_easy_setopt(curl, CURLOPT_URL, API_URL "/createScenario");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStrBufCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->response);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    mtx_lock(&info->mutex);
    info->finished = false;
    mtx_unlock(&info->mutex);

    CURLcode res = curl_easy_perform(curl);
    // TODO: set return value properly here
    if (res != CURLE_OK) {
        goto bail;
    }

bail:
    curl_easy_cleanup(curl);
    mtx_lock(&info->mutex);
    info->finished = true;
    mtx_unlock(&info->mutex);
    return 0;
}

int submitScenario(SubmitScenarioInfo *info) {
    thrd_t threadId;
    int res = thrd_create(&threadId, submitScenarioThread, info);
    if (res != thrd_success) {
        return -1;
    }
    thrd_detach(threadId);
}
