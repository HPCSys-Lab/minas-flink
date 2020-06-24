#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <err.h>
#include <string.h>

#include "./minas.h"
#include "./loadenv.h"

// #define SQR_DISTANCE 1

#ifndef MAIN
#define MAIN
int main(int argc, char *argv[], char **envp) {
    char *executable = argv[0];
    char *modelCsv, *examplesCsv, *matchesCsv, *timingLog;
    FILE *modelFile, *examplesFile, *matches, *timing;
    #define VARS_SIZE 4
    char *varNames[] = { "MODEL_CSV", "EXAMPLES_CSV", "MATCHES_CSV", "TIMING_LOG"};
    char **fileNames[] = { &modelCsv, &examplesCsv, &matchesCsv, &timingLog };
    FILE **files[] = { &modelFile, &examplesFile, &matches, &timing };
    char *fileModes[] = { "r", "r", "w", "a" };
    loadEnv(argc, argv, envp, VARS_SIZE, varNames, fileNames, files, fileModes);
    printf(
        "Reading model from     '%s'\n"
        "Reading examples from  '%s'\n"
        "Writing matches to     '%s'\n"
        "Writing timing to      '%s'\n",
        modelCsv, examplesCsv, matchesCsv, timingLog
    );
    //
    
    Model model;
    model.dimension = 22;
    //
    readModel(model.dimension, modelFile, &model, timing, executable);
    Point *examples;
    int nExamples;
    examples = readExamples(model.dimension, examplesFile, &nExamples, timing, executable);
    // fprintf(stderr, "nExamples %d\n", nExamples);

    fprintf(matches, "#id,isMach,clusterId,label,distance,radius\n");
    int exampleCounter = 0;
    clock_t start = clock();
    Match match;
    Point *unkBuffer = malloc(nExamples * sizeof(Point));
    int nUnk = 0;
    for (exampleCounter = 0; exampleCounter < nExamples; exampleCounter++) {
        // fprintf(stderr, "%d/%d\n", exampleCounter, nExamples);
        //
        classify(model.dimension, &model, &(examples[exampleCounter]), &match);
        if (match.isMatch == 'n') {
            // unkown
            unkBuffer[nUnk] = examples[exampleCounter];
            nUnk++;
        }
        if (nUnk > 100 && nUnk % 100 == 0) {
            // retrain
            Model m;
            m.dimension = 22;
            m.size = 10;
            m.vals = malloc(m.size * sizeof(Cluster));
            for (int i = 0; i < m.size; i++) {
                m.vals[i].id = i;
                m.vals[i].center = malloc(m.dimension * sizeof(double));
            }
            // kMeans(&m, m.size, dimension, unkBuffer, nUnk, timing, executable);
        }
        fprintf(matches, "%d,%c,%d,%c,%e,%e\n",
            match.pointId, match.isMatch, match.clusterId,
            match.label, match.distance, match.radius
        );
    }
    PRINT_TIMING(timing, executable, 1, start, exampleCounter);

    closeEnv(VARS_SIZE, varNames, fileNames, files, fileModes);

    for (int i = 0; i < model.size; i++) {
        free(model.vals[i].center);
    }
    free(model.vals);
    free(examples);
    return 0;
}
#endif // MAIN
