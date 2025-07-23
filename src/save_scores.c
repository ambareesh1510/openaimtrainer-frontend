#include "save_scores.h"

#include <limits.h>

#include "shader.h"

bool savedScoresModified = true;
cvector_vector_type(SavedScore) scoreFiles = NULL;

bool scenarioScoresModified = true;
cvector_vector_type(ScoreSample) scoreSamples = NULL;

int compareStrings(const void *a, const void *b) {
    return strcmp(*(char **) a, *(char **) b);
}

void saveScore(ScenarioMetadata metadata, SavedScore savedScore) {
    const char *scenarioDirPath = GetDirectoryPath(metadata.path);
    const char *scoresPath = TextFormat("%s/scores", scenarioDirPath);
    if (!DirectoryExists(scoresPath)) {
        MakeDirectory(scoresPath);
    }

    // Compute checksums
    int scriptDataSize;
    char *scriptData = LoadFileData(metadata.path, &scriptDataSize);
    int infoDataSize;
    char *infoData = LoadFileData(TextFormat("%s/info.toml", scenarioDirPath), &infoDataSize);
    // TODO: redundant memcpy
    unsigned int scriptChecksum[4];
    unsigned int infoChecksum[4];
    memcpy(scriptChecksum, ComputeMD5(scriptData, scriptDataSize), 4 * sizeof(unsigned int));
    memcpy(infoChecksum, ComputeMD5(infoData, infoDataSize), 4 * sizeof(unsigned int));

    memcpy(savedScore.scriptChecksum, scriptChecksum, 4 * sizeof(unsigned int));
    memcpy(savedScore.infoChecksum, infoChecksum, 4 * sizeof(unsigned int));
    UnloadFileData(scriptData);
    UnloadFileData(infoData);

    time_t currentTime = time(NULL);
    savedScore.time = (intmax_t) currentTime;
    const char *scoreFile = TextFormat("%s/%jd", scoresPath, (intmax_t) currentTime);
    SaveFileData(scoreFile, &savedScore, sizeof(SavedScore));

    // TODO: this should be moved to loadSavedScores
    FilePathList scoreFileList = LoadDirectoryFiles(scoresPath);
    int excess = scoreFileList.count - maxSavedScores;
    if (excess > 0) {
        qsort(scoreFileList.paths, scoreFileList.count, sizeof(scoreFileList.paths[0]), compareStrings);
        for (size_t i = 0; i < excess; i++) {
            remove(scoreFileList.paths[i]);
        }
    }
    UnloadDirectoryFiles(scoreFileList);
}

int compareSavedScores(const void *a, const void *b) {
    SavedScore *x = (SavedScore *) a;
    SavedScore *y = (SavedScore *) b;
    return x->time - y->time;
}

// TODO: make this non-blocking?
void loadSavedScores(ScenarioMetadata metadata) {
    cvector_clear(scoreFiles);

    const char *scenarioDirPath = GetDirectoryPath(metadata.path);
    const char *scoresPath = TextFormat("%s/scores", scenarioDirPath);
    if (!DirectoryExists(scoresPath)) {
        return;
    }

    // Compute checksums
    int scriptDataSize;
    char *scriptData = LoadFileData(metadata.path, &scriptDataSize);
    int infoDataSize;
    char *infoData = LoadFileData(TextFormat("%s/info.toml", scenarioDirPath), &infoDataSize);
    unsigned int scriptChecksum[4];
    unsigned int infoChecksum[4];
    memcpy(scriptChecksum, ComputeMD5(scriptData, scriptDataSize), 4 * sizeof(unsigned int));
    memcpy(infoChecksum, ComputeMD5(infoData, infoDataSize), 4 * sizeof(unsigned int));
    UnloadFileData(scriptData);
    UnloadFileData(infoData);

    FilePathList scoreFileList = LoadDirectoryFiles(scoresPath);
    int scoreFileSize;
    for (size_t i = 0; i < scoreFileList.count; i++) {
        char *rawFileData = LoadFileData(
            scoreFileList.paths[i],
            &scoreFileSize
        );
        SavedScore *scoreFileData = (SavedScore *) rawFileData;
        if (scoreFileSize != sizeof(SavedScore)) {
            goto bail_loop;
        }
        if (strcmp(scoreFileData->magic, SAVED_SCORE_MAGIC) != 0) {
            goto bail_loop;
        }
        if (memcmp(scoreFileData->scriptChecksum, scriptChecksum, 4 * sizeof(unsigned int)) != 0) {
            goto bail_loop;
        }
        if (memcmp(scoreFileData->infoChecksum, infoChecksum, 4 * sizeof(unsigned int)) != 0) {
            goto bail_loop;
        }
        SavedScore newScore;
        memcpy(&newScore, scoreFileData, sizeof(SavedScore));
        cvector_push_back(scoreFiles, newScore);
bail_loop:
        UnloadFileData(rawFileData);
    }
    UnloadDirectoryFiles(scoreFileList);
    qsort(scoreFiles, cvector_size(scoreFiles), sizeof(SavedScore), compareSavedScores);
    savedScoresModified = true;
}

void drawGraph(
    RenderTexture2D texture,
    CustomLayoutElementType type,
    Color clearColor,
    Font font
) {
    int w = texture.texture.width;
    int h = texture.texture.height;
    int cw = texture.texture.width / 2;
    int ch = texture.texture.height / 2;

    float fontSize = 15.0;
    float spacing = 1.0;

    initShaders();
    BeginTextureMode(texture);

    if (scoreFiles == NULL || cvector_size(scoreFiles) == 0) {
        const char *text = "No data";
        // const char *text = "Score";
        Vector2 textSize = MeasureTextEx(font, text, fontSize * 3, spacing);
        DrawTextEx(
            font,
            text,
            (Vector2) {
                cw - textSize.x / 2,
                ch - textSize.y / 2,
            },
            fontSize * 3,
            spacing,
            WHITE
        );
        goto bail;
    }

    int marginX = 65;
    int marginY = 55;
    int topMarginY = 20;

    Color textColor = WHITE;
    Color auxColor = { 255, 255, 255, 100 };
    Color scoreColor = GREEN;
    Color accuracyColor = YELLOW;

    int numSubdivisions = 5;

    DrawRectangle(0, 0, w, h, clearColor);

    DrawRectangle(marginX - 15, 0, 2, h - marginY + 10, auxColor);

    DrawRectangle(w - (marginX - 15), 0, 2, h - marginY + 10, auxColor);

    Vector2 scoreLegendSize = MeasureTextEx(font, "Score", fontSize, spacing);
    Vector2 accuracyLegendSize = MeasureTextEx(font, "Accuracy", fontSize, spacing);

    int labelMid = topMarginY
                + (int) ((h - marginY - topMarginY) / 2);

    DrawTextPro(
        font,
        "Score",
        (Vector2) {
            5,
            labelMid + scoreLegendSize.x  / 2,
        },
        (Vector2) { 0, 0 },
        -90,
        fontSize,
        spacing,
        WHITE
    );

    DrawTextPro(
        font,
        "Accuracy",
        (Vector2) {
            w - 5 - accuracyLegendSize.y,
            labelMid + accuracyLegendSize.x  / 2,
        },
        (Vector2) { 0, 0 },
        -90,
        fontSize,
        spacing,
        WHITE
    );


    int legendLineLength = 30;
    int smallGap = 10;
    int largeGap = 40;

    int legendWidth =
        legendLineLength + smallGap + scoreLegendSize.x
        + largeGap
        + legendLineLength + smallGap + accuracyLegendSize.x;

    int scoreOffset = cw - legendWidth / 2;
    int accuracyOffset = scoreOffset + legendLineLength + smallGap + scoreLegendSize.x + largeGap;

    int legendYOffset = 20;

    DrawRectangle(scoreOffset, h - legendYOffset, legendLineLength, 1, scoreColor);
    DrawCircle(scoreOffset + legendLineLength / 2, h - legendYOffset, 3, scoreColor);

    DrawTextEx(
        font,
        "Score",
        (Vector2) {
            scoreOffset + legendLineLength + smallGap,
            h - legendYOffset - scoreLegendSize.y / 2
        },
        fontSize,
        spacing,
        WHITE
    );


    DrawRectangle(
        accuracyOffset,
        h - legendYOffset,
        legendLineLength,
        1,
        accuracyColor
    );
    DrawCircle(accuracyOffset + legendLineLength / 2, h - legendYOffset, 3, accuracyColor);

    DrawTextEx(
        font,
        "Accuracy",
        (Vector2) {
            accuracyOffset + legendLineLength + smallGap,
            h - legendYOffset - accuracyLegendSize.y / 2
        },
        fontSize,
        spacing,
        WHITE
    );


    int minTime = scoreFiles[0].time;
    int maxTime = scoreFiles[cvector_size(scoreFiles) - 1].time;

    int minScore = INT_MAX, maxScore = 0;
    if (type == DRAW_PROGRESSION_GRAPH) {
        for (size_t i = 0; i < cvector_size(scoreFiles); i++) {
            minScore = MIN(scoreFiles[i].score, minScore);
            maxScore = MAX(scoreFiles[i].score, maxScore);
        }
    } else if (type == DRAW_SCENARIO_GRAPH) {
        for (size_t i = 0; i < cvector_size(scoreSamples); i++) {
            minScore = MIN(scoreSamples[i].score, minScore);
            maxScore = MAX(scoreSamples[i].score, maxScore);
        }
    }

    int bound = 0;
    if (type == DRAW_PROGRESSION_GRAPH) {
        bound = cvector_size(scoreFiles);
    } else if (type == DRAW_SCENARIO_GRAPH) {
        bound = cvector_size(scoreSamples);
    }

    for (size_t i = 0; i <= numSubdivisions; i++) {
        const char *textLeft = TextFormat("%.1f", (minScore + (maxScore - minScore) * (float) i / numSubdivisions));
        const char *textRight = TextFormat("%.0f", 100.0f * i / numSubdivisions);
        Vector2 textSize = MeasureTextEx(font, textLeft, fontSize, spacing);
        if (bound != 1 || i == (numSubdivisions + 1) / 2) {
            DrawTextEx(
                font,
                textLeft,
                (Vector2) {
                    marginX - 15 - textSize.x - 5,
                    topMarginY
                        - textSize.y / 2
                        + (int) ((h - marginY - topMarginY) * (float) (numSubdivisions - i) / numSubdivisions)
                },
                fontSize,
                spacing,
                WHITE
            );
        }
        DrawTextEx(
            font,
            textRight,
            (Vector2) {
                w - (marginX - 20),
                topMarginY
                    - textSize.y / 2
                    + (int) ((h - marginY - topMarginY) * (float) (numSubdivisions - i) / numSubdivisions)
            },
            fontSize,
            spacing,
            WHITE
        );
        DrawRectangle(
            marginX - 15,
            topMarginY
                + (int) ((h - marginY - topMarginY) * (float) (numSubdivisions - i) / numSubdivisions),
            w - 2 * (marginX - 15),
            1,
            (Color) { 255, 255, 255, 30 }
        );
    }

    if (type == DRAW_PROGRESSION_GRAPH) {
        for (size_t i = 0; i <= 1; i++) {
            int maxSize = sizeof("YYYY-MM-DD");
            char *text = malloc(maxSize);
            time_t currTime = (time_t) (minTime + (maxTime - minTime) * i / numSubdivisions);
            struct tm *localTime = localtime(&currTime);
            strftime(text, maxSize, "%m-%d", localTime);
            Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
            DrawTextEx(
                font,
                text,
                (Vector2) {
                    marginX + (int) ((float) (w - 2 * marginX) * i) - textSize.x / 2,
                    h - marginY + 10,
                },
                fontSize,
                spacing,
                WHITE
            );
        }
    }

    if (type == DRAW_SCENARIO_GRAPH) {
        for (size_t i = 0; i <= numSubdivisions; i++) {
            int upperBound = cvector_size(scoreSamples) - 1;
            const char *text = TextFormat("%.1f", (float) upperBound * i / numSubdivisions);
            Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
            DrawTextEx(
                font,
                text,
                (Vector2) {
                    marginX + (int) ((float) (w - 2 * marginX) * i / numSubdivisions) - textSize.x / 2,
                    h - marginY + 10,
                },
                fontSize,
                spacing,
                WHITE
            );
        }
    }

    int prevX, prevScoreY, prevAccuracyY;

    if (bound == 1) {
        int pointY = topMarginY
            + (int) ((h - marginY - topMarginY) * (numSubdivisions - (numSubdivisions + 1) / 2) / numSubdivisions);
        DrawCircle(cw, pointY, 3, accuracyColor);
        DrawCircle(cw, pointY, 3, scoreColor);
    }

    for (size_t i = 0; i < bound; i++) {
        int currScore, currAccuracy;
        if (type == DRAW_PROGRESSION_GRAPH) {
            SavedScore score = scoreFiles[i];
            currScore = score.score;
            currAccuracy = score.accuracy;
        } else if (type == DRAW_SCENARIO_GRAPH) {
            ScoreSample sample = scoreSamples[i];
            currScore = sample.score;
            currAccuracy = sample.accuracy;
        }
        int pointX = marginX + (int) ((w - 2 * marginX) * (float) i / (bound - 1));

        int scoreY = h - (marginY + (int) (
            (float) (h - marginY - topMarginY)
            * (float) (currScore - minScore)
            / (float) (maxScore - minScore)
        ));

        int accuracyY = h - (marginY + (int) (
            (float) (h - marginY - topMarginY)
            * currAccuracy / 100.0f
        ));

        DrawCircle(pointX, accuracyY, 3, accuracyColor);
        DrawCircle(pointX, scoreY, 3, scoreColor);

        if (i != 0) {
            DrawLineEx(
                (Vector2) { prevX, prevAccuracyY },
                (Vector2) { pointX, accuracyY},
                1.0f,
                accuracyColor
            );

            DrawLineEx(
                (Vector2) { prevX, prevScoreY },
                (Vector2) { pointX, scoreY},
                1.0f,
                scoreColor
            );
        }

        prevX = pointX;
        prevScoreY = scoreY;
        prevAccuracyY = accuracyY;
    }

bail:
    EndTextureMode();
}
