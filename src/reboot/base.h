#ifndef _BASE_H
#define _BASE_H 1

#include <err.h>

#define fail(text, args) \
    errx(EXIT_FAILURE, text". At "__FILE__":%d\n", args, __LINE__)

#define assert(exp) \
    if (!(exp)) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)

#define assertMsg(exp, text, args) \
    if (!(exp)) errx(EXIT_FAILURE, "Assert error. " text " At "__FILE__":%d\n", args, __LINE__)

#define PARAMS kParam, dim, precision, radiusF, minExamplesPerCluster, noveltyF
#define PARAMS_ARG int kParam, int dim, double precision, double radiusF, int minExamplesPerCluster, double noveltyF

typedef struct t_example {
    unsigned int id;
    char label;
    double *val;
} Example;

typedef struct {
    unsigned int id, n_matches;
    char label;
    double *center;
    double *ls_valLinearSum, *ss_valSquareSum;
    // double *valAverage, *valStdDev;
    double distanceLinearSum, distanceSquareSum, distanceMax;
    double timeLinearSum, timeSquareSum;
    double distanceAvg, distanceStdDev, radius;
    // double time_mu_μ, time_sigma_σ;
    // assumed last m arrivals in each micro-cluster to be __m = n__
    // so, the m/(2 · n)-th percentile is the 50th percentile
    // therefore z-indez is 0.0 and time_relevance_stamp is the mean distance;
    // double time_relevance_stamp_50;
    // unsigned int *ids, idSize; // used only to reconstruct clusters from snapshot
} Cluster;

typedef struct {
    Cluster *clusters;
    unsigned int size;
    unsigned int nextLabel;
} Model;

typedef struct {
    int pointId, clusterId;
    // char clusterLabel, clusterCatergoy;
    // double clusterRadius;
    char label;
    double distance; // , secondDistance;
    Cluster *cluster;
    // Example *example;
    char *labelStr;
} Match;

#define UNK_LABEL '-'

char *printableLabel(char label);
double nearestClusterVal(int dim, Cluster clusters[], size_t nClusters, double val[], Cluster **nearest);
Cluster* kMeansInit(int kParam, int dim, Example trainingSet[], unsigned int trainingSetSize, unsigned int initalId);
double kMeans(int kParam, int dim, double precision, Cluster* clusters, Example trainingSet[], unsigned int trainingSetSize);
Cluster* clustering(int kParam, int dim, double precision, double radiusF, Example trainingSet[], unsigned int trainingSetSize, unsigned int initalId);
Model *training(int kParam, int dim, double precision, double radiusF);
Match *identify(int kParam, int dim, double precision, double radiusF, Model *model, Example *example, Match *match);

#endif // _BASE_H