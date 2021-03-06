#ifndef MINAS_H
#define MINAS_H 1
#include <stdio.h>

typedef struct point {
    int id;
    // double value[22];
    double *value;
    char label;
} Point;

#define EXAMPLE_CSV_HEADER "#value0,value1,value2,value3,value4,value5,value6,value7,value8,value9,"\
    "value10,value11,value12,value13,value14,value15,value16,value17,value18,value19,"\
    "value20,value21,label\n"
#define EXAMPLE_CSV_LINE_FORMAT "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%c\n"
#define EXAMPLE_CSV_LINE_PRINT_ARGS(ex) \
    ex.value[0], ex.value[1], ex.value[2], ex.value[3], ex.value[4] \
    ex.value[5], ex.value[6], ex.value[7], ex.value[8], ex.value[9] \
    ex.value[10], ex.value[11], ex.value[12], ex.value[13], ex.value[14] \
    ex.value[15], ex.value[16], ex.value[17], ex.value[18], ex.value[19] \
    ex.value[20], ex.value[21], ex.label
#define EXAMPLE_CSV_LINE_SCAN_ARGS(match)                                               \
    &(ex.value[0]), &(ex.value[1]), &(ex.value[2]), &(ex.value[3]), &(ex.value[4]) \
    &(ex.value[5]), &(ex.value[6]), &(ex.value[7]), &(ex.value[8]), &(ex.value[9]) \
    &(ex.value[10]), &(ex.value[11]), &(ex.value[12]), &(ex.value[13]), &(ex.value[14]) \
    &(ex.value[15]), &(ex.value[16]), &(ex.value[17]), &(ex.value[18]), &(ex.value[19]) \
    &(ex.value[20]), &(ex.value[21]), &(ex.label)

/** -------------------------------------------------------------------------------------------------------------------- */

typedef struct cluster {
    int id, matches;
    char label, category;
    double radius, meanDistance, *center;
    double distancesSum, distancesSqrSum, distancesMax;
    double *pointSum, *pointSqrSum;
    int time;
} Cluster;

/** -------------------------------------------------------------------------------------------------------------------- */

typedef struct match {
    int pointId, clusterId;
    char clusterLabel, clusterCatergoy;
    double clusterRadius;
    char label;
    double distance, secondDistance;
    Cluster *cluster;
} Match;

#define MATCH_CSV_HEADER "#pointId,clusterLabel,clusterCategory,clusterId,clusterRadius,label,distance,secondDistance\n"
#define MATCH_CSV_LINE_FORMAT "%10d,%c,%c,%10d,%le,%c,%le,%le\n"
#define MATCH_CSV_LINE_PRINT_ARGS(match) \
    match.pointId, match.clusterLabel, match.clusterCatergoy, match.clusterId, match.clusterRadius, match.label, match.distance, match.secondDistance

/** -------------------------------------------------------------------------------------------------------------------- */

typedef struct model {
    Cluster* vals;
    int size, dimension;
    char nextNovelty;
    Point *unknowns;
    size_t unknownsSize;
    Match *memMatches;
    size_t memMatchesSize;
} Model;

// #ifndef MINAS_FUNCS
// #define MINAS_FUNCS
// double MNS_distance(double a[], double b[], int dimension);
// #endif // MINAS_FUNCS
double MNS_distance(double a[], double b[], int dimension);
Model *readModel(int dimension, FILE *file, FILE *timing, char *executable);
size_t writeModel(FILE *file, Model *model, FILE *timing, char *executable);
Point *readExamples(int dimension, FILE *file, int *nExamples, FILE *timing, char *executable);
// void classify(int dimension, Model *model, Point *ex, Match *match, double allDistances[]);
void classify(int dimension, Model *model, Point *ex, Match *match);

int MNS_minas_main(int argc, char *argv[], char **envp);
Model *MNS_offline(int nExamples, Point examples[], int nClusters, int dimension, FILE *timing, char *executable);
Model *noveltyDetection(int k, Model *model, int unknownsSize, Point unknowns[], int minExCluster, double noveltyThreshold, FILE *timing, char *executable);

#endif // MINAS_H
