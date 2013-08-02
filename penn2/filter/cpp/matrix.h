#ifndef MATRIX_H
#define MATRIX_H

#include <armadillo>

arma::cube repslices(arma::mat matrix, int n_slices);
arma::cube blkdiag(arma::cube A, arma::cube B);
arma::mat blkdiag(arma::mat A, arma::mat B);

#endif // MATRIX_H
