#include <armadillo>

cube repslices(mat matrix, int n_slices)
{
    cube answer = zeros<cube>(matrix.n_rows, matrix.n_cols, n_slices);
    for(int i = 0; i < n_slices; i++)
    {
        answer.slice(i) = matrix;
    }
    
   return answer;
}