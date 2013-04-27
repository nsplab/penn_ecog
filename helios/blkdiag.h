#ifndef BLKDIAG_H
#define BLKDIAG_H

#include <armadillo>
using namespace arma;

// make a block diagnal matrix/cube out of A and B [A 0; 0 B]
mat blkdiag(mat A, mat B);
cube blkdiag(cube A, cube B);

#endif
