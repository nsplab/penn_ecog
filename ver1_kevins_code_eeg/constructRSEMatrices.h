#ifndef CONSTRUCTRSEMATRICES_H
#define CONSTRUCTRSEMATRICES_H

#include <armadillo>

struct RSEMatrixStruct {
    cube F, Q, b;
};

RSEMatrixStruct constructRSEMatrices(int T, mat F, mat Q, mat mean_T, \
    mat Pi_T, int nExtraTimeSteps);

#endif