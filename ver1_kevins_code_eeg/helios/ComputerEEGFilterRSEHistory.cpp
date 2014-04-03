
#include <string>
#include <iostream>
#include <map>
#include <ComputerEEGFilterRSE.h>
#include <parameters.h>
#include <blkdiag.h>
#include <randomStartLocation.h>
#include <constructRSEMatrices.h>
#include <generateChannelParameters.h>
#include <repslices.h>

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
    mat x = join_cols(join_cols( \
        reshape(channelParametersHat, 3 * N_LAGS * N_CHANNELS, 1, 1), pos), \
        uHistory);

    // Select the correct matrices.
    mat F_current = F.slice(timeStep);
    mat Q_current = Q.slice(timeStep);
    mat b_current = b.slice(timeStep);

    // PERFORM THE PREDICTION STEP.
    mat pred_x = F_current * x + b_current;
    mat pred_cov = F_current * covariance * F_current.t() + Q_current;
        
    // PERFORM THE UPDATE STEP.
    mat predicted_u = pred_x.submat(3 * N_LAGS * N_CHANNELS + 3, 0, \
        pred_x.n_rows - 1, 0);
    mat estimated_obs = channelParametersHat * predicted_u;

    // The derivative of the observation vector with respect to the state,
    // evaluated at estimated_obs.
    mat D_obs = zeros<mat>(pred_x.n_rows, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        D_obs.submat(0, c, 3 * N_LAGS * N_CHANNELS - 1, c) = \
            repmat(predicted_u, N_CHANNELS, 1);
        D_obs.submat(D_obs.n_rows - 3 * N_LAGS, c, D_obs.n_rows - 1, c) = \
            pred_x.submat(3 * N_LAGS * c, 0, 3 * N_LAGS * (c + 1) - 1, 0);
    }

    // The double derivative of the observation vector with respect to the
    // state, evaluated at estimated_obs.
    cube DD_obs = zeros<cube>(pred_x.n_rows, pred_x.n_cols, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        for(int i = 0; i<3*N_LAGS; i++)
        {
            DD_obs(3 * N_LAGS * c + i, DD_obs.n_cols - 3 * N_LAGS + i, c) = 1;
            DD_obs(DD_obs.n_rows - 3 * N_LAGS + i, 3 * N_LAGS * c + i, c) = 1;
        }
    }
    
    // What we add to the inverse of the predicted value of the covariance to
    // obtain the updated value.
    mat cov_adjust = pred_cov.zeros();
    for(int c = 0; c < N_CHANNELS; c++)
    {
        cov_adjust += 1 / channelVariances(c) * \
            (trans(D_obs.col(c)) * D_obs.col(c) + \
            (estimated_obs(c, 0) - obs(c)) * DD_obs.slice(c));
    }
    mat new_cov_inv = inv(pred_cov) + cov_adjust;

    /* Check to see if new_cov_inv is well-conditioned. */
    vec singular_values = svd(new_cov_inv);
    mat new_cov;
    double rcond = \
        singular_values(0) / singular_values(singular_values.n_elem - 1);
    if(rcond < 1.0e-8)
        new_cov = inv(new_cov_inv);
    else
    {
        std::cout<<"Matrix is ill-conditioned: "<<rcond<<"; using predicted"\
            <<" covariance instead"<<std::endl;
        new_cov = pred_cov;
    }

    // What we add to the predicted value of the state to obtain the updated
    // value.
    mat x_adjust = pred_x.zeros();
    for(int c = 0; c < N_CHANNELS; c++)
    {
        x_adjust += 1 / channelVariances(c) * \
            (obs(c) - estimated_obs(c, 0)) * D_obs.col(c);
    }
    mat new_x = pred_x + new_cov * x_adjust;

    // UPDATE THE CLASS VARIABLES
    mat new_channelParametersHat = new_x.submat(0, 0, \
                                                3 * N_LAGS * N_CHANNELS - 1, 0);
    channelParametersHat = \
        trans(reshape(new_channelParametersHat, 3 * N_LAGS, N_CHANNELS));
    pos = new_x.submat(3 * N_LAGS * N_CHANNELS, 0, \
        3 * N_LAGS * N_CHANNELS + 2, 0);
    uHistory = new_x.submat(3 * N_LAGS * N_CHANNELS + 3, 0, \
        new_x.n_rows - 1, 0);
    if(covReset.compare("posOnly") == 0)
        covariance = blkdiag(blkdiag(\
            new_cov.submat(0, 0, 3 * N_LAGS * N_CHANNELS - 1, \
            3 * N_LAGS * N_CHANNELS - 1), initialArmCov.submat(0, 0, 2, 2)), \
            new_cov.submat(new_cov.n_rows - 3 * N_LAGS, \
            new_cov.n_cols - 3 * N_LAGS, new_cov.n_rows - 1, \
            new_cov.n_cols - 1));
    else if(covReset.compare("no"))
        covariance = new_cov;
        
    /* I should consider adding a "yes" option to covReset for both position and
       velocity. */
    
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
    
    return returnMap;
}

mat ComputerEEGFilterRSE::newTrial() {
    pos = randomStartLocation();
    uHistory = zeros<mat>(3, N_LAGS);
    
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
