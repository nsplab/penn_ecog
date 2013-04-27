#include "reachstateequation.h"

using namespace arma;

reachStateEquation::reachStateEquation(size_t maxTimeSteps, size_t reachTimeSteps, mat reachTarget)
{
    size_t nExtraTimeSteps = maxTimeSteps - reachTimeSteps;

    mat F = prepareF_ARM_UNDIRECTED();

    // Precompute the inverse and inverse transpose of F.
    vec singular_values_test = svd(F);
    cout << singular_values_test.t() << endl;
    mat F_inv = inv(F);
    mat F_inv_t = trans(F_inv);

    mat Pi_T = prepareREACH_TARGET_COVARIANCE();

    // Precompute all the Pi(t, reachTimeSteps) and phi(t, reachTimeSteps) values.
    cube Pi_t_T = zeros<cube>(Pi_T.n_rows, Pi_T.n_cols, reachTimeSteps + 1);
    cube phi_t_T = zeros<cube>(F.n_rows, F.n_cols, reachTimeSteps + 1);
    phi_t_T.slice(reachTimeSteps) = phi_t_T.slice(reachTimeSteps).eye();

    mat Q = prepareQ_ARM_UNDIRECTED();

    // Eq. 15
    Pi_t_T.slice(reachTimeSteps) = Pi_T + Q;

    for(int t = reachTimeSteps - 1; t >= 0; t--)
    {
        // Eq. 17
        Pi_t_T.slice(t) = F_inv * Pi_t_T.slice(t + 1) * F_inv_t + Q;
        // Eq. 14
        phi_t_T.slice(t) = F_inv * phi_t_T.slice(t + 1);
    }

    // Compute the RSE values for the increment covariance matrix Q.
    cube new_Q = zeros<cube>(Q.n_rows, Q.n_cols, reachTimeSteps);
    // Eq. 23
    for(int t = reachTimeSteps - 1; t >= 0; t--)
    {
        new_Q.slice(t) = Q - Q * inv(Pi_t_T.slice(t)) * Q.t();
    }

    // Compute b_prefix, which is only useful as an intermediate value, as well
    // as the values for the state evolution matrix F.
    cube b_prefix = zeros<cube>(F.n_rows, F.n_cols, reachTimeSteps);
    cube new_F = zeros<cube>(F.n_rows, F.n_cols, reachTimeSteps);
    for(int t = 0; t < reachTimeSteps; t++)
    {
        b_prefix.slice(t) = Q * inv(Pi_t_T.slice(t));
        new_F.slice(t) = F - b_prefix.slice(t) * F;
    }

    // Compute the RSE values of b, which are only nonzero if the target is
    // somewhere other than the origin.
    cube b = zeros<cube>(reachTarget.n_rows, 1, reachTimeSteps);
    // Eq. 28
    for(int t = 0; t < reachTimeSteps; t++)
    {
        b.slice(t) = Q * inv(Pi_t_T.slice(t)) * phi_t_T.slice(t) * reachTarget;
    }

    // Pad the cubes with extra matrices that are used after the duration of the
    // reach is over.
    // Eq. ?
    new_Q = join_slices(new_Q, repslices(Q, nExtraTimeSteps));
    new_F = join_slices(new_F, repslices(F, nExtraTimeSteps));
    b = join_slices(b, repslices(zeros<mat>(b.n_rows, b.n_cols), \
        nExtraTimeSteps));

    // Add all the new matrices to the data structure
    answer_.F = new_F;
    answer_.Q = new_Q;
    answer_.b = b;
}

// Eq. 10
mat reachStateEquation::prepareF_ARM_UNDIRECTED()
{
    mat ans = eye<mat>(6, 6);
    ans(0, 3) = timeBin;
    ans(1, 4) = timeBin;
    ans(2, 5) = timeBin;

    return ans;
}

// Eq. 11
mat reachStateEquation::prepareQ_ARM_UNDIRECTED()
{
    double velInc = 1.0e-4 / timeBin;

    mat ans = zeros<mat>(6, 6);
    ans(3, 3) = velInc;
    ans(4, 4) = velInc;
    ans(5, 5) = velInc;

    return ans;
}

// Eq. 24
mat reachStateEquation::prepareREACH_TARGET_COVARIANCE()
{
    double finalPosCov = 1.0e-6;
    double finalVelCov = 1.0e-8;

    mat ans = zeros<mat>(6, 6);
    ans(0, 0) = finalPosCov;
    ans(1, 1) = finalPosCov;
    ans(2, 2) = finalPosCov;
    ans(3, 3) = finalVelCov;
    ans(4, 4) = finalVelCov;
    ans(5, 5) = finalVelCov;

    return ans;
}


cube reachStateEquation::repslices(mat matrix, int n_slices)
{
    cube answer = zeros<cube>(matrix.n_rows, matrix.n_cols, n_slices);
    for(int i = 0; i < n_slices; i++)
    {
        answer.slice(i) = matrix;
    }

   return answer;
}
