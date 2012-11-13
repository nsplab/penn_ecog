#define _USE_MATH_DEFINES
#include <math.h>
#include <armadillo>
#include "parameters.h"

using namespace arma;

mat randomStartLocation() {
    double startTheta = 2 * M_PI * ((double)rand()/(double)RAND_MAX);
    double startPhi = 2 * M_PI * ((double)rand()/(double)RAND_MAX) - M_PI;
    mat start;
//    start << (START_MAG * cos(startTheta) * sin(startPhi)) << endr << \
//             (START_MAG * sin(startTheta) * sin(startPhi)) << endr << \
//             (START_MAG * cos(startPhi)) << endr;
    start << (START_MAG * cos(startTheta))<< endr << \
             0.0 << endr << \
            (START_MAG * sin(startTheta)) << endr;
    return start;
}
