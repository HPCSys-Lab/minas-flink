#ifndef MINAS_FUNCS
#define MINAS_FUNCS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <err.h>
#include <string.h>

#include "./strsep.c"
#include "./minas.h"

// #define SQR_DISTANCE 1

double MNS_distance(float a[], float b[], int dimension) {
    double distance = 0;
    for (int i = 0; i < dimension; i++) {
        float diff = a[i] - b[i];
        distance += diff * diff;
    }
    #ifdef SQR_DISTANCE
        return distance;
    #else
        return sqrt(distance);
    #endif // SQR_DISTANCE
}

int MNS_classify(Model* model, Point *example, Match *match, int dimension) {
    // since most problems are in the range [0,1], max distance is sqrt(dimesion)
    match->distance = (float) dimension;
    match->pointId = example->id;
    for (int i = 0; i < model->size; i++) {
        float distance = MNS_distance(model->vals[i].center, example->value, dimension);
        if (match->distance > distance) {
            match->clusterId = model->vals[i].id;
            match->label = model->vals[i].label;
            match->radius = model->vals[i].radius;
            match->distance = distance;
        }
    }
    /*
    printf(
        "classify x_%d [0]=%f => min=%e, max=%e, (c0=%e, c=%d, r=%e, m=%d, l=%c)\n",
        example->id, example->value[0], min, max, minCluster->center[0], minCluster->id,
        minCluster->radius, min <= minCluster->radius, minCluster->label
    );
    */
    match->isMatch = match->distance <= match->radius ? 'y' : 'n';
    return match->isMatch;
}

int MNS_printFloatArr(float* value, int dimension) {
    if (value == NULL) return 0;
    int pr = 0;
    for (int i = 0; i < dimension; i++) {
        pr += printf("%f, ", value[i]);
    }
    return pr;
}
int MNS_printPoint(Point *point, int dimension) {
    if (point == NULL) return 0;
    int pr = printf("Point(id=%d, value=[", point->id);
    pr += MNS_printFloatArr(point->value, dimension);
    pr += printf("])\n");
    return pr;
}
int MNS_printCluster(Cluster *cl, int dimension) {
    if (cl == NULL) return 0;
    int pr = printf("Cluster(id=%d, lbl=%c, tm=%d, r=%f, center=[", cl->id, cl->label, cl->time, cl->radius);
    pr += MNS_printFloatArr(cl->center, dimension);
    pr += printf("])\n");
    return pr;
}
int MNS_printModel(Model* model, int dimension) {
    char *labels;
    labels = (char *) malloc(model->size * 3 * sizeof(char));
    for (int i = 0; i < model->size * 3; i += 3){
        labels[i] = model->vals[i].label;
        labels[i + 1] = ',';
        labels[i + 2] = ' ';
    }
    int pr = printf("Model(size=%d, labels=[%s])\n", model->size, labels);
    free(labels);
    for (int i = 0; i < model->size; i++){
        pr += MNS_printCluster(&(model->vals[i]), dimension);
    }
    return pr;
}

Model *MNS_readModelFile(const char *filename, int dimension) {
    clock_t start = clock();
    fprintf(stderr, "Reading model from \t%-30s\n", filename);
    //
    FILE *modelFile = fopen(filename, "r");
    if (modelFile == NULL) {
        errx(EXIT_FAILURE, "bad file open '%s'", filename);
    }
    Model *model = malloc(sizeof(Model));
    model->vals = malloc(1 * sizeof(Cluster));
    // char *separators = strdup("\n, []");
    #define line_len 10 * 1024
    char line[line_len];
    model->size = 0;
    while (fgets(line, line_len, modelFile)) {
        if (*line == '#') continue;
        model->vals = realloc(model->vals, (++model->size) * sizeof(Cluster));
        Cluster *cl = &(model->vals[model->size - 1]);
        if (cl->center == NULL) {
            cl->center = malloc(dimension * sizeof(float));
        }
        int assigned = sscanf(line,
            "%d,%c,%c,"
            "%d,%d,%f,%f,"
            "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            &cl->id, &cl->label, &cl->category,
            &cl->matches, &cl->time, &cl->meanDistance, &cl->radius,
            &cl->center[0], &cl->center[1], &cl->center[2], &cl->center[3], &cl->center[4],
            &cl->center[5], &cl->center[6], &cl->center[7], &cl->center[8], &cl->center[9],
            &cl->center[10], &cl->center[11], &cl->center[12], &cl->center[13], &cl->center[14],
            &cl->center[15], &cl->center[16], &cl->center[17], &cl->center[18], &cl->center[19],
            &cl->center[20], &cl->center[21]
        );
        if (assigned != 29) {
            break;
        }
        #ifdef SQR_DISTANCE
            cl->radius *= cl->radius;
        #endif // SQR_DISTANCE
    }
    fclose(modelFile);
    fprintf(stderr, "Model read took \t%-30fs\n", ((double)(clock() - start)) / ((double)1000000));
    return model;
}

/**
 * Fills an Point with the string format
 * 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,A
 */
int MNS_readExample(Point *ex, FILE *file) {
    #define line_len 10 * 1024
    char line[line_len];
    if (feof(file) || fgets(line, line_len, file) == 0) return 0;
    // while (line[0] == '#') if (!fgets(line, line_len, file)) return 0;
    int assigned = sscanf(line,
        "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"
        "%c\n",
        &ex->value[0], &ex->value[1], &ex->value[2], &ex->value[3], &ex->value[4],
        &ex->value[5], &ex->value[6], &ex->value[7], &ex->value[8], &ex->value[9],
        &ex->value[10], &ex->value[11], &ex->value[12], &ex->value[13], &ex->value[14],
        &ex->value[15], &ex->value[16], &ex->value[17], &ex->value[18], &ex->value[19],
        &ex->value[20], &ex->value[21], &ex->label
    );
    ex->id++;
    return !feof(file) && (assigned == 23);
}

int MNS_classifier() {
    char *modelName = "datasets/model-clean.csv";
    char *testName = "datasets/test.csv";
    #ifdef SQR_DISTANCE
        fprintf(stderr, "Using Square distance (d²)\n");
    #endif // SQR_DISTANCE
    int dimension = 22;
    clock_t start = clock();
    //
    Model *model;
    model = MNS_readModelFile(modelName, dimension);
    //
    Point example;
    example.value = malloc(dimension * sizeof(float));
    example.id = -1;
    Match match;
    //
    fprintf(stderr, "Reading test from %30s\n", testName);
    FILE *kyotoOnl = fopen(testName, "r");
    if (kyotoOnl == NULL) errx(EXIT_FAILURE, "bad file open '%s'", testName);
    // 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,A
    printf("#id,isMach,clusterId,label,distance,radius\n");
    while (MNS_readExample(&example, kyotoOnl)) {
        MNS_classify(model, &example, &match, dimension);
        
        printf("%d,%c,%d,%c,%f,%f\n",
            match.pointId, match.isMatch, match.clusterId,
            match.label, match.distance, match.radius);
    }
    fclose(kyotoOnl);
    // MNS_classifier(model, argv[2]);
    fprintf(stderr, "Total examples \t%d =? 653456\n", example.id);

    for (int i = 0; i < model->size; i++) {
        free(model->vals[i].center);
    }
    free(model->vals);
    free(model);
    fprintf(stderr, "Done %s in \t%fs\n", __FUNCTION__, ((double)(clock() - start)) / ((double)1000000));
    exit(EXIT_SUCCESS);
}

#ifndef MAIN
int main(int argc, char const *argv[]) {
    MNS_classifier();
    return 0;
}
#endif // !MAIN

#endif // MINAS_FUNCS