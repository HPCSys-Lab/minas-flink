typedef struct point
{
    unsigned int id;
    float *value;
} Point;

typedef struct cluster
{
    unsigned int id;
    char label;
    float* center;
    float radius;
    clock_t lastTMS;
} Cluster;

typedef struct model {
    Cluster* vals;
    int size;
} Model;

typedef struct match
{
    unsigned int pointId;
    unsigned int clusterId;
    char isMatch;
    char label;
    float distance;
    float radius;
} Match;

int MNS_dimesion;