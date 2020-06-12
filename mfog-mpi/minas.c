#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <err.h>
#include <string.h>

#include "./minas.h"
#include "./loadenv.h"

// #define SQR_DISTANCE 1

int MNS_classifier(int argc, char *argv[], char **envp) {
    char *executable = argv[0];
    char *modelCsv, *examplesCsv, *matchesCsv, *timingLog;
    FILE *modelFile, *examplesFile, *matches, *timing;
    #define VARS_SIZE 4
    loadEnv(argc, argv, envp, VARS_SIZE,
        (char*[]) {"MODEL_CSV", "EXAMPLES_CSV", "MATCHES_CSV", "TIMING_LOG"},
        (char**[]) { &modelCsv, &examplesCsv, &matchesCsv, &timingLog },
        (FILE**[]) { &modelFile, &examplesFile, &matches, &timing },
        (char*[]) { "r", "r", "w", "a" }
    );
    printf(
        "Reading examples from  '%s'\n"
        "Reading model from     '%s'\n"
        "Writing matches to     '%s'\n"
        "Writing timing to      '%s'\n",
        examplesCsv, modelCsv, matchesCsv, timingLog
    );
    //
    
    #ifdef SQR_DISTANCE
        printf(stderr, "Using Square distance (d²)\n");
    #endif // SQR_DISTANCE
    int dimension = 22;
    //
    Model model;
    readModel(dimension, modelFile, &model, timing, executable);
    Point *examples;
    examples = readExamples(dimension, examplesFile, timing, executable);

    fprintf(matches, "#id,isMach,clusterId,label,distance,radius\n");
    int exampleCounter = 0;
    clock_t start = clock();
    Match match;
    for (exampleCounter = 0; examples[exampleCounter].value != NULL; exampleCounter++) {
        classify(dimension, &model, &(examples[exampleCounter]), &match);
        fprintf(matches, "%d,%c,%d,%c,%e,%e\n",
            match.pointId, match.isMatch, match.clusterId,
            match.label, match.distance, match.radius
        );
    }
    // # source, executable, build_date-time, wall-clock, function, elapsed, cores
    double elapsed = ((double)(clock() - start)) / 1000000.0;
    fprintf(timing, "%s,%s,%s %s,%ld,%s,%e,%d\n",
            __FILE__, executable, __DATE__, __TIME__, time(NULL), __FUNCTION__, elapsed, 1);
    fclose(timing);

    for (int i = 0; i < model.size; i++) {
        free(model.vals[i].center);
    }
    free(model.vals);
    free(examples);
    return 0;
}

#ifndef MAIN
#define MAIN
int main(int argc, char *argv[], char **envp) {
    return MNS_classifier(argc, argv, envp);
}
#endif // MAIN
