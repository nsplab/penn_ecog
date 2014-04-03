
#include <string>
#include <iostream>
#include <map>
#include <math.h>
#include "ComputerEEGFilterCursorOnly.h"
#include "parameters.h"
#include "blkdiag.h"
#include "randomStartLocation.h"
#include "generateChannelParameters.h"
#include "repslices.h"

/* Initializes a new ComputerEEGFilter with parameters read in from
   parameters.cpp. */
ComputerEEGFilterCursorOnly::ComputerEEGFilterCursorOnly() {
    channelParametersHat = generateChannelParameters();
    pos = zeros<mat>(3, 1);
    uHistory = zeros<mat>(3 * N_LAGS, 1);
    initialArmCov = INITIAL_ARM_COV;
    channelVariances = CHANNEL_VARIANCES;
    covReset = COV_RESET;
    
    covariance = initialArmCov;
    F = repslices(F_ARM_UNDIRECTED, MAX_TIME_STEPS);
    b = repslices(zeros<mat>(covariance.n_rows, 1), MAX_TIME_STEPS);
    Q = repslices(Q_ARM_UNDIRECTED, MAX_TIME_STEPS);

    cout<<"matrices replicated"<<endl;

    timeBin = TIME_BIN;
    timeStep = -1;
}

std::map<std::string, mat> ComputerEEGFilterCursorOnly::filter(vec const &obs) {
    timeStep++;

    //cout<<"preparing to initialize test filter state"<<endl;
    // INITIALIZE THE STATE VECTOR. The elements are position (x-y-z) and
    // decoded velocity history, presented in order of x-y-z directionality,
    // then lag number.
    mat prev_u;
    prev_u << uHistory(0, 0) << endr << uHistory(N_LAGS, 0) << endr \
           << uHistory(2 * N_LAGS, 0) << endr;
    mat x = join_cols(pos, prev_u);

    // Select the correct matrices.
    mat F_current = F.slice(timeStep);
    mat Q_current = Q.slice(timeStep);
    mat b_current = b.slice(timeStep);

    //cout<<"initialization performed"<<endl;

    // PERFORM THE PREDICTION STEP.
    mat pred_x = F_current * x + b_current;
    mat pred_cov = F_current * covariance * F_current.t() + Q_current;

    //cout<<"prediction performed"<<endl;
        
    // PERFORM THE UPDATE STEP.
    mat predicted_uHistory = zeros<mat>(uHistory.n_rows, uHistory.n_cols);
    predicted_uHistory(0, 0) = pred_x(3, 0);
    predicted_uHistory.submat(1, 0, N_LAGS - 1, 0) = \
            uHistory.submat(0, 0, N_LAGS - 2, 0);
    predicted_uHistory(N_LAGS, 0) = pred_x(4, 0);
    predicted_uHistory.submat(N_LAGS + 1, 0, 2 * N_LAGS - 1, 0) = \
            uHistory.submat(N_LAGS, 0, 2 * N_LAGS - 2, 0);
    predicted_uHistory(2 * N_LAGS, 0) = pred_x(5, 0);
    predicted_uHistory.submat(2 * N_LAGS + 1, 0, 3 * N_LAGS - 1, 0) = \
            uHistory.submat(2 * N_LAGS, 0, 3 * N_LAGS - 2, 0);

    mat estimated_obs = channelParametersHat * predicted_uHistory;

    //cout<<"observations predicted"<<endl;

    // The derivative of the observation vector with respect to the state,
    // evaluated at estimated_obs.
    mat D_obs = zeros<mat>(pred_x.n_rows, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        D_obs(3, c) = channelParametersHat(c, 0);
        D_obs(4, c) = channelParametersHat(c, N_LAGS);
        D_obs(5, c) = channelParametersHat(c, 2 * N_LAGS);
    }

    //cout<<"D_obs calculated"<<endl;

    // The double derivative of the observation vector with respect to the
    // state, evaluated at estimated_obs.
    cube DD_obs = zeros<cube>(pred_x.n_rows, pred_x.n_rows, N_CHANNELS);
    
    // What we add to the inverse of the predicted value of the covariance to
    // obtain the updated value.
    mat cov_adjust = zeros(pred_cov.n_rows, pred_cov.n_cols);
    for(int c = 0; c < N_CHANNELS; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
            cov_adjust += 1 / channelVariances(c) * \
                (D_obs.col(c) * trans(D_obs.col(c)) + \
                (estimated_obs(c, 0) - obs(c)) * DD_obs.slice(c));
    }
    mat new_cov_inv = inv(pred_cov) + cov_adjust;

    //cout<<"new_cov_inv calculated"<<endl;

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

    //cout<<"new_cov calculated"<<endl;

    // What we add to the predicted value of the state to obtain the updated
    // value.
    mat x_adjust = zeros(pred_x.n_rows, pred_x.n_cols);
    mat innovation = zeros(N_CHANNELS, 1);
    for(int c = 0; c < N_CHANNELS; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
        {
            x_adjust += 1 / channelVariances(c) * \
                (obs(c) - estimated_obs(c, 0)) * D_obs.col(c);
            innovation(c, 0) = \
                    (obs(c) - estimated_obs(c)) / channelVariances(c);
        }
    }
    mat new_x = pred_x + new_cov * x_adjust;

    //cout<<"new_x calculated"<<endl;

    // UPDATE THE CLASS VARIABLES
    pos = new_x.submat(0, 0, 2, 0);

    uHistory = predicted_uHistory;
    uHistory(0, 0) = new_x(3, 0);
    uHistory(N_LAGS, 0) = new_x(4, 0);
    uHistory(2 * N_LAGS, 0) = new_x(5, 0);

    if(covReset.compare("yes") == 0)
        covariance = initialArmCov;
    else if(covReset.compare("posOnly") == 0)
        covariance = blkdiag(initialArmCov.submat(0, 0, 2, 2), \
                             new_cov.submat(3, 3, 5, 5));
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

mat ComputerEEGFilterCursorOnly::newTrial() {
    pos = randomStartLocation();
    uHistory = zeros<mat>(3 * N_LAGS, 1);
    
    covariance = initialArmCov;

    timeStep = -1;
    
    return pos;
}

mat ComputerEEGFilterCursorOnly::newTrial(mat startPos) {
    pos = startPos;
    uHistory = zeros<mat>(3 * N_LAGS, 1);

    covariance = initialArmCov;

    timeStep = -1;

    return pos;
}

mat ComputerEEGFilterCursorOnly::getChannelParameters()
{
    return channelParametersHat;
}

void ComputerEEGFilterCursorOnly::setChannelParameters(\
    mat newChannelParameters) {
        
    channelParametersHat = newChannelParameters;
}
