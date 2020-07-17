#include <stdlib.h>
#include <mpi.h>
#include <err.h>
#include <time.h>

#include "./minas/minas.h"
#include "./mpi/minas-mpi.h"
#include "./util/loadenv.h"

#ifndef MAIN
#define MAIN
int main(int argc, char *argv[], char **envp) {
    int mpiReturn;
    mpiReturn = MPI_Init(&argc, &argv);
    if (mpiReturn != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, mpiReturn);
        errx(EXIT_FAILURE, "MPI Abort %d\n", mpiReturn);
    }
    int clRank, clSize;
    mpiReturn = MPI_Comm_size(MPI_COMM_WORLD, &clSize);
    mpiReturn = MPI_Comm_rank(MPI_COMM_WORLD, &clRank);
    if (mpiReturn != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, mpiReturn);
        errx(EXIT_FAILURE, "MPI Abort %d\n", mpiReturn);
    }
    printf("MPI rank / size => %d/%d\n", clRank, clSize);
    // printEnvs(argc, argv, envp);
    char *executable = argv[0];
    int envSize = 6;
    char                    *trainingCsv,   *modelCsv,      *modelOutCsv,       *examplesCsv,   *matchesCsv,    *timingLog;
    FILE                    *trainingFile,  *modelFile,     *modelOutFile,      *examplesFile,  *matches,       *timing;
    char *varNames[] = {    "TRAINING_CSV", "MODEL_CSV",    "MODEL_OUT_CSV",    "EXAMPLES_CSV", "MATCHES_CSV",  "TIMING_LOG" };
    char **fileNames[] = {  &trainingCsv,   &modelCsv,      &modelOutCsv,       &examplesCsv,   &matchesCsv,    &timingLog };
    FILE **files[] = {      &trainingFile,  &modelFile,     &modelOutFile,      &examplesFile,  &matches,       &timing };
    char *fileModes[] = {   "r",            "r",            "w",                "r",            "w",            "a" };
    if (clRank == 0) {
        loadEnv(argc, argv, envp, envSize, varNames, fileNames, files, fileModes);
        printf(
            "Reading training from  '%s'\n"
            "Reading model from     '%s'\n"
            "Writing model out to   '%s'\n"
            "Reading examples from  '%s'\n"
            "Writing matches to     '%s'\n"
            "Writing timing to      '%s'\n",
            trainingCsv, modelCsv, modelOutCsv, examplesCsv, matchesCsv, timingLog
        );
    }
    //
    Model *model;
    if (clRank == 0) {
        if (trainingCsv != NULL && trainingFile != NULL) {
            // printf("will training\n");
            int nExamples;
            Point *examples = readExamples(22, trainingFile, &nExamples, timing, executable);
            model = MNS_offline(nExamples, examples, 100, 22, timing, executable);
            if (modelOutFile != NULL) {
                writeModel(model->dimension, modelOutFile, model, timing, executable);
            }
        } else if (modelCsv != NULL && modelFile != NULL) {
            model = readModel(22, modelFile, timing, executable);
        }
    }
    Point *examples;
    Match *memMatches;
    int nExamples;
    if (examplesCsv != NULL && examplesFile != NULL) {
        examples = readExamples(model->dimension, examplesFile, &nExamples, timing, executable);
        memMatches = malloc(nExamples * sizeof(Match));
    }
    if (clSize == 1) {
        clock_t start = clock();
        for (int i = 0; i < nExamples; i++) {
            classify(model->dimension, model, &(examples[i]), &(memMatches[i]));
        }
        PRINT_TIMING(timing, executable, clSize, start, nExamples);
        fprintf(matches, MATCH_CSV_HEADER);
        for (int i = 0; i < nExamples; i++) {
            fprintf(matches, MATCH_CSV_LINE_FORMAT, MATCH_CSV_LINE_PRINT_ARGS(memMatches[i]));
        }
    } else if (clRank == 0) {
        MPI_Barrier(MPI_COMM_WORLD);
        sendModel(model->dimension, model, clRank, clSize, timing, executable);
        
        MPI_Barrier(MPI_COMM_WORLD);
        clock_t start = clock();
        int exampleCounter = sendExamples(model->dimension, examples, memMatches, clSize, timing, executable);

        MPI_Barrier(MPI_COMM_WORLD);
        PRINT_TIMING(timing, executable, clSize, start, exampleCounter);
        fprintf(matches, MATCH_CSV_HEADER);
        for (int i = 0; i < exampleCounter; i++) {
            fprintf(matches, MATCH_CSV_LINE_FORMAT, MATCH_CSV_LINE_PRINT_ARGS(memMatches[i]));
        }
        closeEnv(envSize, varNames, fileNames, files, fileModes);
    } else {
        MPI_Barrier(MPI_COMM_WORLD);
        model = malloc(sizeof(Model));
        receiveModel(0, model, clRank);
        MPI_Barrier(MPI_COMM_WORLD);

        receiveExamples(model->dimension, model, clRank);

        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    free(model);
    free(examples);
    free(memMatches);
    return 0;
}
#endif // MAIN