#ifndef REPSLICES_H
#define REPSLICES_H

#include <armadillo>
using namespace arma;

// copies a matrix in a cube n_slices times
cube repslices(mat matrix, int n_slices);

#endif
