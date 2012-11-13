#include <armadillo>
using namespace arma;

mat blkdiag(mat A, mat B) {
    mat C = zeros<mat>(A.n_rows + B.n_rows, A.n_cols + B.n_cols);

    C.submat(0, 0, A.n_rows - 1, A.n_cols - 1) = A;
    C.submat(A.n_rows, A.n_cols, C.n_rows - 1, C.n_cols - 1) = B;

    return C;
}

cube blkdiag(cube A, cube B) {
    cube C = zeros<cube>(A.n_rows + B.n_rows, A.n_cols + B.n_cols, A.n_slices);

    C.subcube(0, 0, 0, A.n_rows - 1, A.n_cols - 1, A.n_slices - 1) = A;
    C.subcube(A.n_rows, A.n_cols, 0, C.n_rows - 1, C.n_cols - 1, \
        C.n_slices - 1) = B;

    return C;
}
