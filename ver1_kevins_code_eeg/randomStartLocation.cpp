#define _USE_MATH_DEFINES
#include <math.h>
#include <armadillo>
#include <parameters.h>

mat randomStartLocation() {
    double startAngle = 2 * M_PI * ((double)rand()/(double)RAND_MAX);
    mat start;
    start << (START_MAG * cos(startAngle)) << endr << \
        (START_MAG * sin(startAngle)) << endr;
    return start;
}