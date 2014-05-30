#include "reachstateequation.h"
#include "matrix.h"

// Equation numbers are relative to "Dynamic Analysis of Naive Adaptive Brain-Machine Interfaces"
using namespace arma;

reachStateEquation::reachStateEquation(size_t maxTimeSteps, size_t reachTimeSteps, mat reachTarget, size_t dim, double diagQ, double finalPosCov, double finalVelCov, double timeBin)
{
    timeBin_ = timeBin;
    size_t nExtraTimeSteps = maxTimeSteps - reachTimeSteps;

    mat F = prepareF_ARM_UNDIRECTED(dim);

    // Precompute the inverse and inverse transpose of F.
    vec singular_values_test = svd(F);
    //cout << singular_values_test.t() << endl;
    mat F_inv = inv(F);
    mat F_inv_t = trans(F_inv);

    mat Pi_T = prepareREACH_TARGET_COVARIANCE(dim, finalPosCov, finalVelCov);

    // Precompute all the Pi(t, reachTimeSteps) and phi(t, reachTimeSteps) values.
    cube Pi_t_T = zeros<cube>(Pi_T.n_rows, Pi_T.n_cols, reachTimeSteps + 1);
    cube phi_t_T = zeros<cube>(F.n_rows, F.n_cols, reachTimeSteps + 1);
    phi_t_T.slice(reachTimeSteps) = phi_t_T.slice(reachTimeSteps).eye();

    mat Q = prepareQ_ARM_UNDIRECTED(dim, diagQ);

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
    //cout << "new_F:\n" << new_F << "\n";
    //cout << "new_Q:\n" << new_Q << "\n";
    //cout << "b:\n" << b << "\n";
    //exit(0);
}

// Eq. 10
mat reachStateEquation::prepareF_ARM_UNDIRECTED(size_t dim)
{
    mat ans = eye<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(i, dim + i) = timeBin_;
    }

    return ans;
}

// Eq. 11
mat reachStateEquation::prepareQ_ARM_UNDIRECTED(size_t dim, double velInc)
{
    //double velInc = 1.0e-4 / timeBin;
    //velInc = 1.0e-3; // this is the used value

    mat ans = zeros<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(dim + i, dim + i) = velInc;
    }

    return ans;
}

// Eq. 24
mat reachStateEquation::prepareREACH_TARGET_COVARIANCE(size_t dim, double finalPosCov, double finalVelCov)
{
    //double finalPosCov = 1.0e-6;
    //double finalVelCov = 1.0e-8;

    mat ans = zeros<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(i, i) = finalPosCov;
        ans(dim + i, dim + i) = finalVelCov;
    }

    return ans;
}






timeInvariantRSE::timeInvariantRSE(mat reachTarget, mat Q, mat R, size_t dim)
{
    timeBin = 0.01;
    mat A = prepareF_ARM_UNDIRECTED(dim);
    for (size_t i = 0; i < dim; i++) {
        // Perfect filter would ignore previous velocity
        A(dim + i, dim + i) = 0;
    }
    cout << "A:\n" << A << "\n";
    mat B = zeros(2 * dim, dim);
    for (size_t i = 0; i < dim; i++) {
        B(1, 0) = 1;
    }

    // Reasonable cost values
    // const double alpha = 0.18;
    // const double beta = 0.1;
    // const double gamma = 0.1;
    // mat Q = zeros<mat>(2 * dim, 2 * dim);
    // for (size_t i = 0; i < dim; i++) {
    //     Q(i, i) = alpha;
    //     Q(dim + i, dim + i) = beta;
    // }
    // mat R = gamma * eye<mat>(dim, dim);

    cout << "B:\n" << B << "\n";
    cout << "Q:\n" << Q << "\n";
    cout << "R:\n" << R << "\n";

    mat K = Q;
    //mat L;
    for (int i = 0; i < 10000; i++) {
        K = trans(A)*(K-K*B*inv(trans(B)*K*B+R)*trans(B)*K)*A+Q;
        //if (i % 100 == 0) {
        //    cout << "K:\t" << i << "\n" << K << "\n";
        //    cout << "L:\n" << L << "\n";
        //}
        //L = -inv(trans(B)*K*B)*trans(B)*K*A;
    }
    mat L = -inv(trans(B)*K*B)*trans(B)*K*A;
    cout << "L:\n" << L << "\n";

    mat new_F = zeros(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        new_F(i, i) = 1;
        new_F(i, dim + i) = timeBin;
    }
    new_F.submat(dim, 0, 2 * dim - 1, 2 * dim - 1) = L;
    mat new_Q = prepareQ_ARM_UNDIRECTED(dim); // TODO: check this
    mat new_b = new_F * reachTarget;
    answer_.F = repslices(new_F, 1);
    answer_.Q = repslices(new_Q, 1);
    answer_.b = repslices(new_b, 1);
    cout << "F:\n" << new_F << "\n";
    cout << "Q:\n" << new_Q << "\n";
    cout << "b:\n" << new_b << "\n";

    //mat x = zeros<mat>(2, 1);
    //x(0, 0) = 10;
    //cout << x << "\n";
    //for (int i = 0; i < 1000; i++) {
    //    mat u = L * x;
    //    x = A * x + B * u;
    //    cout << x << "\n";
    //}

}

// Eq. 10
mat timeInvariantRSE::prepareF_ARM_UNDIRECTED(size_t dim)
{
    mat ans = eye<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(i, dim + i) = timeBin;
    }

    return ans;
}

// Eq. 11
mat timeInvariantRSE::prepareQ_ARM_UNDIRECTED(size_t dim)
{
    double velInc = 1.0e-4 / timeBin;
    velInc = 1.0e-3;

    mat ans = zeros<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(dim + i, dim + i) = velInc;
    }

    return ans;
}

// Eq. 24
mat timeInvariantRSE::prepareREACH_TARGET_COVARIANCE(size_t dim)
{
    double finalPosCov = 1.0e-6;
    double finalVelCov = 1.0e-8;

    mat ans = zeros<mat>(2 * dim, 2 * dim);
    for (size_t i = 0; i < dim; i++) {
        ans(i, i) = finalPosCov;
        ans(dim + i, dim + i) = finalVelCov;
    }

    return ans;
}

