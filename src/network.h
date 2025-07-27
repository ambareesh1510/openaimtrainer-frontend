#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>

#include "curl/curl.h"
#include "tinycthread/tinycthread.h"
#include "cjson/cJSON.h"

void initCurl(void);

struct StrBuf {
    size_t len;
    char *buf;
};
typedef struct StrBuf StrBuf;

void freeStrBuf(StrBuf *data);

extern char *authToken;
extern char *username;

void extractAuthDetails(StrBuf response, char **tokenBuf, char **usernameBuf);

// =================
// Sign up (/signup)
// =================

// Only writes to `inProgress` and `finished` are read concurrently
// and need to be locked.
enum AuthRequestType {
    AUTH_REQUEST_SIGN_UP,
    AUTH_REQUEST_LOG_IN,
};
typedef enum AuthRequestType AuthRequestType;

struct AuthRequestInfo {
    mtx_t mutex;
    AuthRequestType type;
    const char *username;
    const char *email;
    const char *password;
    StrBuf response;
    bool finished;
};
typedef struct AuthRequestInfo AuthRequestInfo;

AuthRequestInfo createAuthRequestInfo(
    AuthRequestType type,
    const char *username,
    const char *email,
    const char *password
);
void cleanupAuthRequestInfo(AuthRequestInfo *info);

int sendAuthRequest(AuthRequestInfo *info);

// =================================
// Submit scenario (/createScenario)
// =================================
struct SubmitScenarioInfo {
    mtx_t mutex;
    const char *name;
    const char *author;
    double time;
    const char *infoPath;
    const char *scriptPath;
    StrBuf response;
    bool finished;
};
typedef struct SubmitScenarioInfo SubmitScenarioInfo;

SubmitScenarioInfo createSubmitScenarioInfo(
    const char *name,
    const char *author,
    double time,
    const char *infoPath,
    const char *scriptPath
);
void cleanupSubmitScenarioInfo(SubmitScenarioInfo *info);

int submitScenario(SubmitScenarioInfo *info);

#endif /* NETWORK_H */
