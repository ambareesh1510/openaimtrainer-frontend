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
    savedScoresModified = true;

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
}

void drawGraph(
    int bboxX,
    int bboxY,
    int bboxW,
    int bboxH,
    CustomLayoutElementType type,
    Color clearColor
) {
    int w = bboxW;
    int h = bboxH;
    int cw = bboxW / 2;
    int ch = bboxH / 2;

    float fontSize = 15.0;
    float spacing = 1.0;

    DrawRectangle(bboxX, bboxY, bboxW, bboxH, clearColor);

    if (scoreFiles == NULL || cvector_size(scoreFiles) == 0) {
        Font largeFont = fonts[FONT_ID_GRAPH];
        const char *text = "No data";
        Vector2 textSize = MeasureTextEx(largeFont, text, 45.0, spacing);
        DrawTextEx(
            largeFont,
            text,
            (Vector2) {
                bboxX + cw - textSize.x / 2,
                bboxY + ch - textSize.y / 2,
            },
            45.0,
            spacing,
            WHITE
        );
        goto bail;
    }

    Font font = fonts[FONT_ID_BODY_16];

    int marginX = 65;
    int marginY = 55;
    int topMarginY = 20;

    Color textColor = WHITE;
    Color auxColor = { 255, 255, 255, 100 };
    Color scoreColor = GREEN;
    Color accuracyColor = YELLOW;

    int numSubdivisions = 5;


    DrawRectangle(bboxX + marginX - 15, bboxY + 10, 2, h - marginY, auxColor);

    DrawRectangle(bboxX + w - (marginX - 15), bboxY + 10, 2, h - marginY, auxColor);

    Vector2 scoreLegendSize = MeasureTextEx(font, "Score", fontSize, spacing);
    Vector2 accuracyLegendSize = MeasureTextEx(font, "Accuracy", fontSize, spacing);

    int labelMid = topMarginY
                + (int) ((h - marginY - topMarginY) / 2);

    DrawTextPro(
        font,
        "Score",
        (Vector2) {
            bboxX + 5,
            bboxY + labelMid + scoreLegendSize.x  / 2,
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
            bboxX + w - 5 - accuracyLegendSize.y,
            bboxY + labelMid + accuracyLegendSize.x  / 2,
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

    DrawRectangle(bboxX + scoreOffset, bboxY + h - legendYOffset, legendLineLength, 1, scoreColor);
    DrawCircle(bboxX + scoreOffset + legendLineLength / 2, bboxY + h - legendYOffset, 3, scoreColor);

    DrawTextEx(
        font,
        "Score",
        (Vector2) {
            bboxX + scoreOffset + legendLineLength + smallGap,
            bboxY + h - legendYOffset - scoreLegendSize.y / 2
        },
        fontSize,
        spacing,
        WHITE
    );


    DrawRectangle(
        bboxX + accuracyOffset,
        bboxY + h - legendYOffset,
        legendLineLength,
        1,
        accuracyColor
    );
    DrawCircle(bboxX + accuracyOffset + legendLineLength / 2, bboxY + h - legendYOffset, 3, accuracyColor);

    DrawTextEx(
        font,
        "Accuracy",
        (Vector2) {
            bboxX + accuracyOffset + legendLineLength + smallGap,
            bboxY + h - legendYOffset - accuracyLegendSize.y / 2
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
                    bboxX + marginX - 15 - textSize.x - 5,
                    bboxY + topMarginY
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
                bboxX + w - (marginX - 20),
                bboxY + topMarginY
                    - textSize.y / 2
                    + (int) ((h - marginY - topMarginY) * (float) (numSubdivisions - i) / numSubdivisions)
            },
            fontSize,
            spacing,
            WHITE
        );
        DrawRectangle(
            bboxX + marginX - 15,
            bboxY + topMarginY
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
                    bboxX + marginX + (int) ((float) (w - 2 * marginX) * i) - textSize.x / 2,
                    bboxY + h - marginY + 10,
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
                    bboxX + marginX + (int) ((float) (w - 2 * marginX) * i / numSubdivisions) - textSize.x / 2,
                    bboxY + h - marginY + 10,
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
        DrawCircle(bboxX + cw, bboxY + pointY, 3, accuracyColor);
        DrawCircle(bboxX + cw, bboxY + pointY, 3, scoreColor);
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

        DrawCircle(bboxX + pointX, bboxY + accuracyY, 3, accuracyColor);
        DrawCircle(bboxX + pointX, bboxY + scoreY, 3, scoreColor);

        if (i != 0) {
            DrawLineEx(
                (Vector2) { bboxX + prevX, bboxY + prevAccuracyY },
                (Vector2) { bboxX + pointX, bboxY + accuracyY},
                1.0f,
                accuracyColor
            );

            DrawLineEx(
                (Vector2) { bboxX + prevX, bboxY + prevScoreY },
                (Vector2) { bboxX + pointX, bboxY + scoreY},
                1.0f,
                scoreColor
            );
        }

        prevX = pointX;
        prevScoreY = scoreY;
        prevAccuracyY = accuracyY;
    }

bail:
}
