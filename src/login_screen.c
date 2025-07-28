#include "login_screen.h"

#include "clay/clay.h"
#include "ui_utils.h"
#include "network.h"

#define USERNAME_PLACEHOLDER "Username"
TextBoxData usernameData = {
    .placeholder = USERNAME_PLACEHOLDER,
    .placeholderLen = sizeof(USERNAME_PLACEHOLDER) - 1,
    .id = 1,
};

#define EMAIL_PLACEHOLDER "Email"
TextBoxData emailData = {
    .placeholder = EMAIL_PLACEHOLDER,
    .placeholderLen = sizeof(EMAIL_PLACEHOLDER) - 1,
    .id = 2,
};

#define PASSWORD_PLACEHOLDER "Password"
TextBoxData passwordData = {
    .placeholder = PASSWORD_PLACEHOLDER,
    .placeholderLen = sizeof(PASSWORD_PLACEHOLDER) - 1,
    .obfuscated = true,
    .id = 3,
};

AuthRequestType currentPageAuthRequestType = AUTH_REQUEST_LOG_IN;
AuthRequestInfo authRequestInfo;

void handleSwitchPageSignUp(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        currentPageAuthRequestType = AUTH_REQUEST_SIGN_UP;
    }
}

void handleSwitchPageLogIn(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        currentPageAuthRequestType = AUTH_REQUEST_LOG_IN;
    }
}

void handleSubmitAuthRequest(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        cleanupAuthRequestInfo(&authRequestInfo);
        authRequestInfo = createAuthRequestInfo(currentPageAuthRequestType, usernameData.str, emailData.str, passwordData.str);
        sendAuthRequest(&authRequestInfo);
    }
}

void renderLoginScreen(void) {
    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = { 16, 16, 16, 16 },
            .childGap = 16,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = COLOR_GRAY,
    }) {
        Clay_String titleText;
        if (currentPageAuthRequestType == AUTH_REQUEST_LOG_IN) {
            titleText = CLAY_STRING("Log In");
        } else if (currentPageAuthRequestType == AUTH_REQUEST_SIGN_UP) {
            titleText = CLAY_STRING("Sign Up");
        }
        CLAY_TEXT(titleText, &titleTextConfig);

        if (currentPageAuthRequestType == AUTH_REQUEST_SIGN_UP) {
            CDIV(CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(50)) {
                renderTextBox(&usernameData);
            }
        }

        CDIV(CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(50)) {
            renderTextBox(&emailData);
        }

        CDIV(CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(50)) {
            renderTextBox(&passwordData);
        }

        bool authParamsValid = (strlen(emailData.str) != 0) && (strlen(passwordData.str) != 0);
        if (currentPageAuthRequestType == AUTH_REQUEST_SIGN_UP) {
            authParamsValid = authParamsValid && (strlen(usernameData.str) != 0);
        }

        bool allowAuthRequest = false;
        if (authParamsValid) {
            if (!authRequestInfo.requestData.dispatched) {
                allowAuthRequest = true;
            } else {
                bool requestFinished;
                mtx_lock(&authRequestInfo.requestData.mutex);
                requestFinished = authRequestInfo.requestData.finished;
                mtx_unlock(&authRequestInfo.requestData.mutex);
                if (requestFinished) {
                    authRequestInfo.requestData.dispatched = false;
                    allowAuthRequest = true;
                }
            }
        }

        Clay_TextElementConfig submitButtonConfig = hugeTextConfig;
        if (!allowAuthRequest) {
            submitButtonConfig.textColor = COLOR_LIGHT_GRAY;
        }

        CLAY({
            .backgroundColor = allowAuthRequest
                ? (Clay_Hovered()
                    ? COLOR_DARK_BLUE
                    : COLOR_LIGHT_GRAY)
                : COLOR_DARK_GRAY,
            .layout = {
                .padding = { 5, 5, 5, 5 },
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3),
                },
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                },
            },
        }) {
            if (allowAuthRequest) {
                Clay_OnHover(handleSubmitAuthRequest, 0);
            }
            CLAY_TEXT(CLAY_STRING("Submit"), CLAY_TEXT_CONFIG(submitButtonConfig));
        }

        if (currentPageAuthRequestType == AUTH_REQUEST_LOG_IN) {
            CLAY({
                .border = {
                    .width = { 0, 0, 0, 1 },
                    .color = COLOR_OFF_WHITE,
                }
            }) {
                Clay_OnHover(handleSwitchPageSignUp, 0);
                CLAY_TEXT(CLAY_STRING("Don't have an account? Click here to create one!"), &normalTextConfig);
            }
        } else if (currentPageAuthRequestType == AUTH_REQUEST_SIGN_UP) {
            CLAY({
                .border = {
                    .width = { 0, 0, 0, 1 },
                    .color = COLOR_OFF_WHITE,
                }
            }) {
                Clay_OnHover(handleSwitchPageLogIn, 0);
                CLAY_TEXT(CLAY_STRING("Already have an account? Click here to log in!"), &normalTextConfig);
            }
        }
        
        // mtx_lock(&authRequestInfo.mutex);
        // bool signUpRequestFinished = authRequestInfo.finished;
        // mtx_unlock(&authRequestInfo.mutex);

        // if (signUpRequestFinished && authRequestInfo.response.buf != NULL) {
        //     CLAY_TEXT(CLAY_DYNSTR(authRequestInfo.response.buf), &largeTextConfig);
        //     printf("Signed in as %s\n", username);
        // }

        // mtx_lock(&submitScenarioInfo.mutex);
        // bool submitScenarioRequestFinished = submitScenarioInfo.finished;
        // mtx_unlock(&submitScenarioInfo.mutex);
        //
        // if (submitScenarioRequestFinished && submitScenarioInfo.response.buf != NULL) {
        //     CLAY_TEXT(CLAY_DYNSTR(submitScenarioInfo.response.buf), &largeTextConfig);
        // }
    }
}
