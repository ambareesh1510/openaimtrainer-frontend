#ifndef SAVE_SCORES_H
#define SAVE_SCORES_H

#include <time.h>
#include "ui_utils.h"

#define SAVED_SCORE_MAGIC "SCORE"
struct SavedScore {
    char magic[sizeof(SAVED_SCORE_MAGIC)];
    unsigned int scriptChecksum[4];
    unsigned int infoChecksum[4];
    intmax_t time;
    int score;
    float accuracy;
};
typedef struct SavedScore SavedScore; 

struct ScoreSample {
    int score;
    float accuracy;
};
typedef struct ScoreSample ScoreSample;

void saveScore(ScenarioMetadata metadata, SavedScore savedScore);

extern bool savedScoresModified;
extern cvector_vector_type(SavedScore) scoreFiles;

extern bool scenarioScoresModified;
extern cvector_vector_type(ScoreSample) scoreSamples;

void loadSavedScores(ScenarioMetadata metadata);

void drawGraph(
    int bboxX,
    int bboxY,
    int bboxW,
    int bboxH,
    CustomLayoutElementType type,
    Color clearColor
);

#endif /* SAVE_SCORES_H */
