#include "save_scores.h"

#include <limits.h>

#include "shader.h"

bool savedScoresModified = false;
cvector_vector_type(SavedScore) scoreFiles = NULL;

bool scenarioScoresModified = false;
cvector_vector_type(ScoreSample) scoreSamples = NULL;

void saveScore(ScenarioMetadata metadata, SavedScore savedScore) {
    const char *scenarioDirPath = GetDirectoryPath(metadata.path);
    const char *scoresPath = TextFormat("%s/scores", scenarioDirPath);
    if (!DirectoryExists(scoresPath)) {
        MakeDirectory(scoresPath);
    }
    time_t currentTime = time(NULL);
    savedScore.time = (intmax_t) currentTime;
    const char *scoreFile = TextFormat("%s/%jd", scoresPath, (intmax_t) currentTime);
    printf("Saving to %s\n", scoreFile);
    SaveFileData(scoreFile, &savedScore, sizeof(SavedScore));
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

    FilePathList scoreFileList = LoadDirectoryFiles(scoresPath);
    int scoreFileSize;
    for (size_t i = 0; i < scoreFileList.count; i++) {
        SavedScore *scoreFileData = (SavedScore *) LoadFileData(
            scoreFileList.paths[i],
            &scoreFileSize
        );
        if (scoreFileSize != sizeof(SavedScore)) {
            goto bail_loop;
        }
        if (strcmp(scoreFileData->magic, SAVED_SCORE_MAGIC) != 0) {
            goto bail_loop;
        }
        SavedScore newScore;
        memcpy(&newScore, scoreFileData, sizeof(SavedScore));
        cvector_push_back(scoreFiles, newScore);
bail_loop:
        free(scoreFileData);
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

    initShaders();
    BeginTextureMode(texture);

    int marginX = 65;
    int marginY = 40;
    int topMarginY = 20;

    Color textColor = WHITE;
    Color auxColor = { 255, 255, 255, 100 };
    Color scoreColor = GREEN;
    Color accuracyColor = YELLOW;

    int numSubdivisions = 5;
    float fontSize = 15.0;
    float spacing = 1.0;

    DrawRectangle(0, 0, w, h, clearColor);

    DrawRectangle(marginX - 15, 0, 2, h, auxColor);

    DrawRectangle(w - (marginX - 15), 0, 2, h, auxColor);

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

    DrawRectangle(scoreOffset, h - marginY / 2, legendLineLength, 1, scoreColor);
    DrawCircle(scoreOffset + legendLineLength / 2, h - marginY / 2, 3, scoreColor);

    DrawTextEx(
        font,
        "Score",
        (Vector2) {
            scoreOffset + legendLineLength + smallGap,
            h - marginY / 2 - scoreLegendSize.y / 2
        },
        fontSize,
        spacing,
        WHITE
    );


    DrawRectangle(
        accuracyOffset,
        h - marginY / 2,
        legendLineLength,
        1,
        accuracyColor
    );
    DrawCircle(accuracyOffset + legendLineLength / 2, h - marginY / 2, 3, accuracyColor);

    DrawTextEx(
        font,
        "Accuracy",
        (Vector2) {
            accuracyOffset + legendLineLength + smallGap,
            h - marginY / 2 - accuracyLegendSize.y / 2
        },
        fontSize,
        spacing,
        WHITE
    );

    if (cvector_size(scoreFiles) == 0) {
        goto bail;
    }

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

    for (size_t i = 0; i <= numSubdivisions; i++) {
        const char *textLeft = TextFormat("%.1f", (minScore + (maxScore - minScore) * (float) i / numSubdivisions));
        const char *textRight = TextFormat("%.0f", 100.0f * i / numSubdivisions);
        Vector2 textSize = MeasureTextEx(font, textLeft, fontSize, spacing);
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

    int prevX, prevScoreY, prevAccuracyY;
    int bound = 0;
    if (type == DRAW_PROGRESSION_GRAPH) {
        bound = cvector_size(scoreFiles);
    } else if (type == DRAW_SCENARIO_GRAPH) {
        bound = cvector_size(scoreSamples);
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
