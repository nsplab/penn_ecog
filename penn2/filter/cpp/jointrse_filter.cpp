#include "jointrse_filter.h"

using namespace arma;

jointRSE_filter::jointRSE_filter():
    timeStep_(-1), prevTrialId_(0)
{
    pos_ = zeros<mat>(3, 1);

    const double timeBin = 0.01;
    // page 31
    const double maxTimeSteps = 3.0/timeBin;
    // page 27
    int reachTimeSteps = 1.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);

    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    rseParams = rseComputer.returnAnswer();

    uHistory_ = zeros<mat>(3 * numLags, 1);

    mat FSingleTimeChannels = eye<mat>(3 * numLags, 3 * numLags);
    cube FChannels = repslices(FSingleTimeChannels, maxTimeSteps);
    // Eq. 18
    F_ = blkdiag(FChannels, rseParams.F);

    mat QSingleTimeChannels = zeros<mat>(3 * numLags, 3 * numLags);
    cube QChannels = repslices(QSingleTimeChannels, maxTimeSteps);
    // Eq.19
    Q_ = blkdiag(QChannels, rseParams.Q);

    const double channelCov = 1.0e-2;
    mat initialChannelCov = channelCov * eye<mat>(3 * numLags, 3 * numLags);
    covariance_ = prepareINITIAL_ARM_COV(timeBin);
    covariance_ = blkdiag(initialChannelCov, covariance_);

    b_ = zeros<cube>(numChannels * FChannels.n_rows + rseParams.b.n_rows, 1, maxTimeSteps);
    b_.subcube(numChannels * FChannels.n_rows, 0, 0, b_.n_rows-1, b_.n_cols-1, b_.n_slices - 1) = rseParams.b;

    channelParametersHat_ = zeros<mat>(numChannels, 3 * numLags);
}

void jointRSE_filter::Predict() {
    timeStep_ ++;

    // INITIALIZE THE STATE VECTOR. The first elements are the channel
    // parameters, presented in order of channel number, then x-y-z
    // directionality (x then y then z), and finally lag number (least
    // lag to greatest lag). Then comes position (x-y-z) and decoded
    // velocity history, presented in order of x-y-z directionality,
    // then lag number.
    // NOTE: The third argument 1 to reshape signifies row-wise reshaping. I
    //       think.
    //cout<<"preparing to initialize state"<<endl;
    mat prev_u;
    //cout<<"uHistory n_rows " << uHistory.n_rows << endl;
    prev_u << uHistory_(0, 0) << endr << uHistory_(numLags, 0) << endr
           << uHistory_(2 * numLags, 0) << endr;
    //cout<<"prev_u successfully initialized"<<endl;
    //cout<<"channelParametersHat n_rows " << channelParametersHat.n_rows << "n_cols " << channelParametersHat.n_cols<< endl;
    mat x = join_cols(join_cols(
        reshape(channelParametersHat_, 3 * numLags * numChannels, 1, 1), pos_),
        prev_u);
    //cout<<"initialized state"<<endl;

    // Select the correct matrices.
    mat F_current = F_.slice(timeStep_);
    mat Q_current = Q_.slice(timeStep_);
    mat b_current = b_.slice(timeStep_);

    // PERFORM THE PREDICTION STEP.
    mat pred_x = F_current * x + b_current;
    mat pred_cov = F_current * covariance_ * F_current.t() + Q_current;
}

void jointRSE_filter::Update() {
    // PERFORM THE UPDATE STEP.
    mat predicted_uHistory = zeros<mat>(uHistory.n_rows, uHistory.n_cols);
    predicted_uHistory(0, 0) = pred_x(3 * N_LAGS * N_CHANNELS + 3, 0);
    predicted_uHistory.submat(1, 0, N_LAGS - 1, 0) = \
            uHistory.submat(0, 0, N_LAGS - 2, 0);
    predicted_uHistory(N_LAGS, 0) = \
            pred_x(3 * N_LAGS * N_CHANNELS + 4, 0);
    predicted_uHistory.submat(N_LAGS + 1, 0, 2 * N_LAGS - 1, 0) = \
            uHistory.submat(N_LAGS, 0, 2 * N_LAGS - 2, 0);
    predicted_uHistory(2 * N_LAGS, 0) = \
            pred_x(3 * N_LAGS * N_CHANNELS + 5, 0);
    predicted_uHistory.submat(2 * N_LAGS + 1, 0, 3 * N_LAGS - 1, 0) = \
            uHistory.submat(2 * N_LAGS, 0, 3 * N_LAGS - 2, 0);

    mat estimated_obs = channelParametersHat * predicted_uHistory;

    // The derivative of the observation vector with respect to the state,
    // evaluated at estimated_obs.
    // Eq. 3.10 (write up)
    mat D_obs = zeros<mat>(pred_x.n_rows, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        D_obs.submat(c * 3 * N_LAGS, c, (c + 1) * 3 * N_LAGS - 1, c) = \
                predicted_uHistory;
        mat pred_pars;
        pred_pars << pred_x(c * 3 * N_LAGS, 0) << endr \
                  << pred_x(c * 3 * N_LAGS + N_LAGS, 0) << endr \
                  << pred_x(c * 3 * N_LAGS + 2 * N_LAGS, 0) << endr;
        D_obs.submat(D_obs.n_rows - 3, c, D_obs.n_rows - 1, c) = pred_pars;
    }

    // The double derivative of the observation vector with respect to the
    // state, evaluated at estimated_obs.
    //cout<<"Computing DD_OBS"<<endl;
    // Eq. 3.13 (write up)
    cube DD_obs = zeros<cube>(pred_x.n_rows, pred_x.n_rows, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        DD_obs(c * 3 * N_LAGS, DD_obs.n_cols - 3, c) = 1;
        DD_obs(c * 3 * N_LAGS + N_LAGS, DD_obs.n_cols - 2, c) = 1;
        DD_obs(c * 3 * N_LAGS + 2 * N_LAGS, DD_obs.n_cols - 1, c) = 1;

        DD_obs(DD_obs.n_rows - 3, c * 3 * N_LAGS, c) = 1;
        DD_obs(DD_obs.n_rows - 2, c * 3 * N_LAGS + N_LAGS, c) = 1;
        DD_obs(DD_obs.n_rows - 1, c * 3 * N_LAGS + 2 * N_LAGS, c) = 1;
    }
    //cout<<"Done computing DD_OBS"<<endl;

    // What we add to the inverse of the predicted value of the covariance to
    // obtain the updated value.
    //cout<<"Computing cov_adjust"<<endl;
    // part of Eq. 3.12 (write up)
    mat cov_adjust = zeros(pred_cov.n_rows, pred_cov.n_cols);
    for(int c = 0; c < N_CHANNELS; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
            cov_adjust += 1 / channelVariances(c) * \
                (D_obs.col(c) * trans(D_obs.col(c)) + \
                (estimated_obs(c, 0) - obs(c)) * DD_obs.slice(c));
    }
    //cout<<"Done computing cov_adjust"<<endl;
    //vec test_singular_values = svd(pred_cov);
    //cout<<"singular values of predicted covariance: "<<test_singular_values<<endl;
    // Eq. 3.12 (write up)
    mat new_cov_inv = inv(pred_cov) + cov_adjust;
    cout<<"Done computing new_cov_inv"<<endl;

    /* Check to see if new_cov_inv is well-conditioned. */
    vec singular_values = svd(new_cov_inv);
    mat new_cov;
    double rcond = \
        singular_values(0) / singular_values(singular_values.n_elem - 1);
    if(rcond >= 1.0e-8)
        new_cov = inv(new_cov_inv);
    else
    {
        std::cout<<"Matrix is ill-conditioned: "<<rcond<<"; using predicted"\
            <<" covariance instead"<<std::endl;
        new_cov = pred_cov;
    }
    //cout<<"done computing new_cov"<<endl;

    // What we add to the predicted value of the state to obtain the updated
    // value.
    //cout<<"Computing x_adjust"<<endl;
    mat x_adjust = zeros(pred_x.n_rows, pred_x.n_cols);
    mat innovation = zeros(N_CHANNELS, 1);
    // part of Eq. 3.11 (write up)
    for(int c = 0; c < N_CHANNELS; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
        {
            x_adjust += 1 / channelVariances(c) * \
                (obs(c) - estimated_obs(c, 0)) * D_obs.col(c);
            innovation(c, 0) = \
                    (obs(c) - estimated_obs(c, 0)) / channelVariances(c);
        }
    }
    //cout<<"Done computing x_adjust"<<endl;
    // Eq. 3.11 (write up)
    mat new_x = pred_x + new_cov * x_adjust;
    //cout<<"Done computing new_x"<<endl;

    // UPDATE THE CLASS VARIABLES
    mat new_channelParametersHat = new_x.submat(0, 0, \
                                                3 * N_LAGS * N_CHANNELS - 1, 0);
    channelParametersHat = \
        trans(reshape(new_channelParametersHat, 3 * N_LAGS, N_CHANNELS, 1));
    pos = new_x.submat(3 * N_LAGS * N_CHANNELS, 0, \
        3 * N_LAGS * N_CHANNELS + 2, 0);

    uHistory = predicted_uHistory;
    uHistory(0, 0) = new_x(3 * N_LAGS * N_CHANNELS + 3, 0);
    uHistory(N_LAGS, 0) = new_x(3 * N_LAGS * N_CHANNELS + 4, 0);
    uHistory(2 * N_LAGS, 0) = new_x(3 * N_LAGS * N_CHANNELS + 5, 0);

    if(covReset.compare("yes") == 0)
        covariance = blkdiag(new_cov.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
            3 * N_LAGS * N_CHANNELS - 1), initialArmCov);
    else if(covReset.compare("posOnly") == 0)
        covariance = blkdiag(blkdiag(\
            new_cov.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
            3 * N_LAGS * N_CHANNELS - 1), initialArmCov.submat(0, 0, 2, 2)), \
            new_cov.submat(new_cov.n_rows - 3, new_cov.n_cols - 3, \
            new_cov.n_rows - 1, new_cov.n_cols - 1));
    else if(covReset.compare("no"))
        covariance = new_cov;
}

void jointRSE_filter::InitNewTrial() {
    pos = startPos;
    uHistory = zeros<mat>(3 * N_LAGS, 1);

    covariance = blkdiag(covariance.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
        3 * N_LAGS * N_CHANNELS - 1), initialArmCov);

    timeStep = -1;
}

void jointRSE_filter::Run() {
    GrabFeatures();
    if (prevTrialId_ != trial_id) {
        InitNewTrial();
        prevTrialId_ = trial_id;
    }
    Predict();
    Update();
    //PublishHandMovement();
}

cube jointRSE_filter::repslices(mat matrix, int n_slices)
{
    cube answer = zeros<cube>(matrix.n_rows, matrix.n_cols, n_slices);
    for(int i = 0; i < n_slices; i++)
    {
        answer.slice(i) = matrix;
    }

   return answer;
}

cube jointRSE_filter::blkdiag(cube A, cube B) {
    cube C = zeros<cube>(A.n_rows + B.n_rows, A.n_cols + B.n_cols, A.n_slices);

    C.subcube(0, 0, 0, A.n_rows - 1, A.n_cols - 1, A.n_slices - 1) = A;
    C.subcube(A.n_rows, A.n_cols, 0, C.n_rows - 1, C.n_cols - 1, \
        C.n_slices - 1) = B;

    return C;
}

mat jointRSE_filter::blkdiag(mat A, mat B) {
    mat C = zeros<mat>(A.n_rows + B.n_rows, A.n_cols + B.n_cols);

    C.submat(0, 0, A.n_rows - 1, A.n_cols - 1) = A;
    C.submat(A.n_rows, A.n_cols, C.n_rows - 1, C.n_cols - 1) = B;

    return C;
}

// The initial covariance on the arm components of the state.
// page 20
mat jointRSE_filter::prepareINITIAL_ARM_COV(const double timeBin)
{
    mat ans = zeros<mat>(6, 6);
         double posCov = 1.0e-7;
         double velCov = 1.0e-7 / timeBin;
         ans(0, 0) = posCov;
         ans(1, 1) = posCov;
         ans(2, 2) = posCov;
         ans(3, 3) = velCov;
         ans(4, 4) = velCov;
         ans(5, 5) = velCov;

        return ans;
}
