#include <constructRSEMatrices.h>
#include <repslices.h>
//Equation numbering is according to [1]
//[1]:
RSEMatrixStruct constructRSEMatrices(int T, mat F, mat Q, mat mean_T, \
    mat Pi_T, int nExtraTimeSteps) {

    // Precompute the inverse and inverse transpose of F.
    F_inv = inv(F);
    F_inv_t = trans(F_inv);
    
    // Precompute all the Pi(t, T) and phi(t, T) values.
    cube Pi_t_T = zeros<cube>(Pi_T.n_rows, Pi_T.n_cols, T + 1);
    cube phi_t_T = zeros<cube>(F.n_rows, F.n_cols, T + 1);
    
    //Equivalent to equation 2.17
    Pi_t_T.slice(T) = Pi_T + Q;
    phi_t_T.slice(T) = phi_t_T.slice(T).eye();
    
    for(int t = T - 1; t >= 0; t--)
    {
        //Equivalent to Equation 2.16 
        Pi_t_T.slice(t) = F_inv * Pi_t_T.slice(t + 1) * F_inv_t + Q; 
        phi_t_T.slice(t) = F_inv * phi_t_T.slice(t + 1);
    }
    
    // Compute the RSE values for the increment covariance matrix Q.
    cube new_Q = zeros<cube>(Q.n_rows, Q.n_cols, T);
    for(int t = T - 1; t >= 0; t--)
    {
        new_Q.slice(t) = Q - Q * inv(Pi_t_T.slice(t)) * Q.t();
    }
    
    // Replace any elements of new_Q that might have turned out negative with 0.
    // NOTE: This is present in the original function, but I'm unsure as to
    //       how important this is. Leaving it out because unsure whether uword
    //       arithmetic plays nice with integers.
//     for(int i = 0; i < new_Q.n_slices; i++)
//     {
//         currentSlice = new_Q.slice(i);
//         uvec negIndices = find(currentSlice < 0);
//         for(j = 0; j < negIndices.n_elem; j++)
//         {
//             col = negIndices(j)
//             if(currentSlice()
//         }
//     }
    
    
    // Compute b_prefix, which is only useful as an intermediate value, as well
    // as the values for the state evolution matrix F.
    cube b_prefix = zeros<cube>(F.n_rows, F.n_cols, T);
    cube new_F = zeros<cube>(F.n_rows, F.n_cols, T);
    for(int t = 0; t < T; t++)
    {
        b_prefix.slice(t) = Q * inv(Pi_t_T.slice(t));
        new_F.slice(t) = F - b_prefix.slice(t) * F;
    }
    
    // Compute the RSE values of b, which are only nonzero if the target is 
    // somewhere other than the origin.
    cube b = zeros<cube>(mean_T.n_rows, 1, T);
    for(int t = 0; t < T; t++)
    {
        //equivalent to equation 2.12
        b.slice(t) = Q * inv(Pi_t_T.slice(t)) * phi_t_T.slice(t) * mean_T;
    }
    
    // Pad the cubes with extra matrices that are used after the duration of the
    // reach is over.
    new_Q = new_Q.join_slices(repslices(Q, nExtraTimeSteps));
    new_F = new_F.join_slices(repslices(F, nExtraTimeSteps));
    b = b.join_slices(repslices(zeros<mat>(b.n_rows, b.n_cols), \
        nExtraTimeSteps));
        
    // Add all the new matrices to the data structure.
    RSEMatrixStruct answer = new RSEMatrixStruct();
    answer.F = new_F;
    answer.Q = new_Q;
    answer.b = b;
    
    return answer;
}
