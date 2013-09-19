#include "matrix.h"

using namespace arma;

cube repslices(mat matrix, int n_slices)
{
    cube answer = zeros<cube>(matrix.n_rows, matrix.n_cols, n_slices);
    for(int i = 0; i < n_slices; i++)
    {
        answer.slice(i) = matrix;
    }

   return answer;
}

cube blkdiag(cube A, cube B) {
    cube C = zeros<cube>(A.n_rows + B.n_rows, A.n_cols + B.n_cols, A.n_slices);

    C.subcube(0, 0, 0, A.n_rows - 1, A.n_cols - 1, A.n_slices - 1) = A;
    C.subcube(A.n_rows, A.n_cols, 0, C.n_rows - 1, C.n_cols - 1,
        C.n_slices - 1) = B;

    return C;
}

mat blkdiag(mat A, mat B) {
    mat C = zeros<mat>(A.n_rows + B.n_rows, A.n_cols + B.n_cols);

    C.submat(0, 0, A.n_rows - 1, A.n_cols - 1) = A;
    C.submat(A.n_rows, A.n_cols, C.n_rows - 1, C.n_cols - 1) = B;

    return C;
}

