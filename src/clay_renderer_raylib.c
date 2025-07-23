#include "clay_renderer_raylib.h"

#include "save_scores.h"
#include "shader.h"

RenderTexture2D settingsCrosshairTexture = { 0 };
bool settingsCrosshairTextureInitialized = false;

RenderTexture2D progressionGraphTexture = { 0 };
bool progressionGraphTextureInitialized = false;

RenderTexture2D scenarioGraphTexture = { 0 };
bool scenarioGraphTextureInitialized = false;

cvector_vector_type(Clay_ScissorData) scissorDataStack = NULL;

Clay_ScissorData clipScissorData(Clay_ScissorData outer, Clay_ScissorData inner) {
    int x = MAX(outer.x, inner.x);
    int y = MAX(outer.y, inner.y);
    int width = MAX(
        MIN(outer.x + outer.width, inner.x + inner.width) - x,
        0
    );
    int height = MAX(
        MIN(outer.y + outer.height, inner.y + inner.height) - y,
        0
    );
    return (Clay_ScissorData) {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .id = inner.id,
    };
}

Ray GetScreenToWorldPointWithZDistance(Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance)
{
    Ray ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*position.x)/(float)screenWidth - 1.0f;
    float y = 1.0f - (2.0f*position.y)/(float)screenHeight;
    float z = 1.0f;

    // Store values in a vector
    Vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)screenWidth/(double)screenHeight), 0.01f, zDistance);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)screenWidth/(double)screenHeight;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    Vector3 nearPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    Vector3 farPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Calculate normalized direction vector
    Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

    ray.position = farPoint;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}


void Clay_Raylib_Initialize(int width, int height, const char *title, unsigned int flags) {
    SetConfigFlags(flags);
    InitWindow(width, height, title);
//    EnableEventWaiting();
}

void Clay_Raylib_Close()
{
    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    CloseWindow();
}

void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts)
{
    cvector_clear(scissorDataStack);
    for (int j = 0; j < renderCommands.length; j++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = {roundf(renderCommand->boundingBox.x), roundf(renderCommand->boundingBox.y), roundf(renderCommand->boundingBox.width), roundf(renderCommand->boundingBox.height)};
        switch (renderCommand->commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData *textData = &renderCommand->renderData.text;
                Font fontToUse = fonts[textData->fontId];
    
                int strlen = textData->stringContents.length + 1;
    
                if(strlen > temp_render_buffer_len) {
                    // Grow the temp buffer if we need a larger string
                    if(temp_render_buffer) free(temp_render_buffer);
                    temp_render_buffer = (char *) malloc(strlen);
                    temp_render_buffer_len = strlen;
                }
    
                // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
                memcpy(temp_render_buffer, textData->stringContents.chars, textData->stringContents.length);
                temp_render_buffer[textData->stringContents.length] = '\0';
                DrawTextEx(fontToUse, temp_render_buffer, (Vector2){boundingBox.x, boundingBox.y}, (float)textData->fontSize, (float)textData->letterSpacing, CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor));
    
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D imageTexture = *(Texture2D *)renderCommand->renderData.image.imageData;
                Clay_Color tintColor = renderCommand->renderData.image.backgroundColor;
                if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 && tintColor.a == 0) {
                    tintColor = (Clay_Color) { 255, 255, 255, 255 };
                }
                DrawTexturePro(
                    imageTexture,
                    (Rectangle) { 0, 0, imageTexture.width, imageTexture.height },
                    (Rectangle){boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height},
                    (Vector2) {},
                    0,
                    CLAY_COLOR_TO_RAYLIB_COLOR(tintColor));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                Clay_ScissorData data = {
                    .x = (int) roundf(boundingBox.x),
                    .y = (int) roundf(boundingBox.y),
                    .width = (int) roundf(boundingBox.width),
                    .height = (int) roundf(boundingBox.height),
                    .id = renderCommand->id,
                };
                if (cvector_size(scissorDataStack) > 0) {
                    data = clipScissorData(
                        *cvector_back(scissorDataStack),
                        data
                    );
                }
                cvector_push_back(scissorDataStack, data);
                BeginScissorMode(
                    data.x,
                    data.y,
                    data.width,
                    data.height
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                cvector_pop_back(scissorDataStack);

                if (cvector_size(scissorDataStack) > 0) {
                    Clay_ScissorData *data = cvector_back(scissorDataStack);
                    BeginScissorMode(
                        data->x,
                        data->y,
                        data->width,
                        data->height
                    );
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleRenderData *config = &renderCommand->renderData.rectangle;
                if (config->cornerRadius.topLeft > 0) {
                    float radius = (config->cornerRadius.topLeft * 2) / (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                    DrawRectangleRounded((Rectangle) { boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }, radius, 8, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                } else {
                    DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData *config = &renderCommand->renderData.border;
                // Left border
                if (config->width.left > 0) {
                    DrawRectangle((int)roundf(boundingBox.x), (int)roundf(boundingBox.y + config->cornerRadius.topLeft), (int)config->width.left, (int)roundf(boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Right border
                if (config->width.right > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + boundingBox.width - config->width.right), (int)roundf(boundingBox.y + config->cornerRadius.topRight), (int)config->width.right, (int)roundf(boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Top border
                if (config->width.top > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.topLeft), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight), (int)config->width.top, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Bottom border
                if (config->width.bottom > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.bottomLeft), (int)roundf(boundingBox.y + boundingBox.height - config->width.bottom), (int)roundf(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight), (int)config->width.bottom, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topLeft > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) }, roundf(config->cornerRadius.topLeft - config->width.top), config->cornerRadius.topLeft, 180, 270, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topRight > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) }, roundf(config->cornerRadius.topRight - config->width.top), config->cornerRadius.topRight, 270, 360, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomLeft > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) }, roundf(config->cornerRadius.bottomLeft - config->width.bottom), config->cornerRadius.bottomLeft, 90, 180, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomRight > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) }, roundf(config->cornerRadius.bottomRight - config->width.bottom), config->cornerRadius.bottomRight, 0.1, 90, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData *config = &renderCommand->renderData.custom;
                CustomLayoutElementData *customData = config->customData;
                if (customData->type == DRAW_CROSSHAIR_TEXTURE) {
                    // TODO: this will break on resize
                    if (!settingsCrosshairTextureInitialized) {
                        settingsCrosshairTexture = LoadRenderTexture(
                            boundingBox.width,
                            boundingBox.height
                        );
                        settingsCrosshairTextureInitialized = true;
                    }

                    drawCrosshair(
                        settingsCrosshairTexture,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                    );
                    DrawTexture(
                        settingsCrosshairTexture.texture,
                        boundingBox.x,
                        boundingBox.y,
                        WHITE
                    );
                } else if (
                    customData->type == DRAW_PROGRESSION_GRAPH
                    || customData->type == DRAW_SCENARIO_GRAPH
                ) {
                    if (
                        customData->type == DRAW_PROGRESSION_GRAPH
                        && (!progressionGraphTextureInitialized || savedScoresModified)
                    ) {
                        UnloadRenderTexture(progressionGraphTexture);
                        progressionGraphTexture = LoadRenderTexture(
                            boundingBox.width,
                            boundingBox.height
                        );
                        progressionGraphTextureInitialized = true;
                    }
                    if (
                        customData->type == DRAW_SCENARIO_GRAPH
                        && (!scenarioGraphTextureInitialized || scenarioScoresModified)
                    ) {
                        UnloadRenderTexture(scenarioGraphTexture);
                        scenarioGraphTexture = LoadRenderTexture(
                            boundingBox.width,
                            boundingBox.height
                        );
                        scenarioGraphTextureInitialized = true;
                    }
                    if (customData->type == DRAW_PROGRESSION_GRAPH && savedScoresModified) {
                        drawGraph(
                            progressionGraphTexture,
                            customData->type,
                            CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor),
                            fonts[0]
                        );
                        savedScoresModified = false;
                    }
                    if (customData->type == DRAW_SCENARIO_GRAPH && scenarioScoresModified) {
                        drawGraph(
                            scenarioGraphTexture,
                            customData->type,
                            CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor),
                            fonts[0]
                        );
                        scenarioScoresModified = false;
                    }
                    if (customData->type == DRAW_PROGRESSION_GRAPH) {
                        DrawTextureRec(
                            progressionGraphTexture.texture,
                            (Rectangle) {
                                0, 0, boundingBox.width, -boundingBox.height
                            },
                            (Vector2) {
                                boundingBox.x,
                                boundingBox.y,
                            },
                            WHITE
                        );
                    } else if (customData->type == DRAW_SCENARIO_GRAPH) {
                        DrawTextureRec(
                            scenarioGraphTexture.texture,
                            (Rectangle) {
                                0, 0, boundingBox.width, -boundingBox.height
                            },
                            (Vector2) {
                                boundingBox.x,
                                boundingBox.y,
                            },
                            WHITE
                        );
                    }
                }
                break;
            }
            default: {
                printf("Error: unhandled render command.");
                exit(1);
            }
        }
    }
}
