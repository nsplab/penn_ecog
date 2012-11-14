#ifndef BLKDIAG_H
#define BLKDIAG_H

#include <armadillo>
using namespace arma;

mat blkdiag(mat A, mat B);
cube blkdiag(cube A, cube B);

#endif
