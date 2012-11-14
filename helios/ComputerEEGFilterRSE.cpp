
#include <string>
#include <iostream>
#include <map>
#include <math.h>
#include "ComputerEEGFilterRSE.h"
#include "parameters.h"
#include "blkdiag.h"
#include "randomStartLocation.h"
#include "constructRSEMatrices.h"
#include "generateChannelParameters.h"
#include "repslices.h"

/* Initializes a new ComputerEEGFilterRSE with parameters read in from
   parameters.cpp. */
ComputerEEGFilterRSE::ComputerEEGFilterRSE() {
    channelParametersHat = generateChannelParameters();
    pos = zeros<mat>(3, 1);
    uHistory = zeros<mat>(3 * N_LAGS, 1);
    initialChannelCov = INITIAL_CHANNEL_COV;
    initialArmCov = INITIAL_ARM_COV;
    channelVariances = CHANNEL_VARIANCES;
    covReset = COV_RESET;
    
    F_channel = repslices(F_CHANNEL, MAX_TIME_STEPS);
    Q_channel = repslices(Q_CHANNEL, MAX_TIME_STEPS);
    
    mat F_undirected = F_ARM_UNDIRECTED;
    mat Q_undirected = Q_ARM_UNDIRECTED;
    
    RSEMatrixStruct matrices = constructRSEMatrices(REACH_TIME_STEPS, \
        F_undirected, Q_undirected, REACH_TARGET, REACH_TARGET_COVARIANCE, \
        MAX_TIME_STEPS - REACH_TIME_STEPS);
    cout<<"RSE matrices created"<<endl;

    F_arm = matrices.F;
    b_arm = matrices.b;
    Q_arm = matrices.Q;

    F = F_arm;
    Q = Q_arm;
    covariance = initialArmCov;
    for(int c=0; c<N_CHANNELS; c++)
    {
        F = blkdiag(F_channel, F);
        Q = blkdiag(Q_channel, Q);
        covariance = blkdiag(initialChannelCov, covariance);
    }
    b = zeros<cube>(N_CHANNELS * F_channel.n_rows + b_arm.n_rows, 1, \
                    MAX_TIME_STEPS);
    b.subcube(N_CHANNELS * F_channel.n_rows, 0, 0, b.n_rows - 1, b.n_cols - 1, \
              b.n_slices - 1) = b_arm;
    cout<<"RSE filter matrices replicated"<<endl;

    timeBin = TIME_BIN;
    timeStep = -1;
}

std::map<std::string, mat> ComputerEEGFilterRSE::filter(vec const &obs) {
    timeStep++;

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
    prev_u << uHistory(0, 0) << endr << uHistory(N_LAGS, 0) << endr \
           << uHistory(2 * N_LAGS, 0) << endr;
    //cout<<"prev_u successfully initialized"<<endl;
    //cout<<"channelParametersHat n_rows " << channelParametersHat.n_rows << \
          "n_cols " << channelParametersHat.n_cols<< endl;
    mat x = join_cols(join_cols( \
        reshape(channelParametersHat, 3 * N_LAGS * N_CHANNELS, 1, 1), pos), \
        prev_u);
    //cout<<"initialized state"<<endl;

    // Select the correct matrices.
    mat F_current = F.slice(timeStep);
    mat Q_current = Q.slice(timeStep);
    mat b_current = b.slice(timeStep);

    // PERFORM THE PREDICTION STEP.
    mat pred_x = F_current * x + b_current;
    mat pred_cov = F_current * covariance * F_current.t() + Q_current;
    //cout<<"covariance diagonal: "<<covariance.diag().t()<<endl;
    //cout<<"predicted covariance: "<<pred_cov.diag().t()<<endl;
        
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
    
    // RETURN THE MAP OF DATA
    std::map<std::string, mat> returnMap;
    returnMap["state"] = new_x;
    returnMap["covariance"] = covariance;
    returnMap["position"] = pos;
    returnMap["channelParameters"] = channelParametersHat;
    mat vel;
    vel << uHistory(0, 0) << endr << uHistory(N_LAGS, 0) << endr << \
        uHistory(2 * N_LAGS, 0) << endr;
    returnMap["velocity"] = vel;
    returnMap["innovation"] = innovation;
    
    return returnMap;
}

mat ComputerEEGFilterRSE::newTrial() {
    pos = randomStartLocation();
    uHistory = zeros<mat>(3 * N_LAGS, 1);
    
    covariance = blkdiag(covariance.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
        3 * N_LAGS * N_CHANNELS - 1), initialArmCov);

    timeStep = -1;
    
    return pos;
}

mat ComputerEEGFilterRSE::newTrial(mat startPos) {
    pos = startPos;
    uHistory = zeros<mat>(3 * N_LAGS, 1);

    covariance = blkdiag(covariance.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
        3 * N_LAGS * N_CHANNELS - 1), initialArmCov);

    timeStep = -1;

    return pos;
}

mat ComputerEEGFilterRSE::getChannelParameters()
{
    return channelParametersHat;
}

void ComputerEEGFilterRSE::setChannelParameters(mat newChannelParameters) {
    channelParametersHat = newChannelParameters;
}
