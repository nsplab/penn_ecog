
#include <string>
#include <iostream>
#include <map>
#include <ComputerEEGFilterCursorOnly.h>
#include <parameters.h>
#include <blkdiag.h>
#include <randomStartLocation.h>
#include <generateChannelParameters.h>
#include <repslices.h>

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

    timeBin = TIME_BIN;
    timeStep = -1;
}

std::map<std::string, mat> ComputerEEGFilterCursorOnly::filter(vec const &obs) {
    timeStep++;

    // INITIALIZE THE STATE VECTOR. The elements are position (x-y-z) and
    // decoded velocity history, presented in order of x-y-z directionality,
    // then lag number.
    mat x = join_cols(pos, uHistory);

    // Select the correct matrices.
    mat F_current = F.slice(timeStep);
    mat Q_current = Q.slice(timeStep);
    mat b_current = b.slice(timeStep);

    // PERFORM THE PREDICTION STEP.
    mat pred_x = F_current * x + b_current;
    mat pred_cov = F_current * covariance * F_current.t() + Q_current;
        
    // PERFORM THE UPDATE STEP.
    mat predicted_u = pred_x.submat(3, 0, pred_x.n_rows - 1, 0);
    mat estimated_obs = channelParametersHat * predicted_u;

    // The derivative of the observation vector with respect to the state,
    // evaluated at estimated_obs.
    mat D_obs = zeros<mat>(pred_x.n_rows, N_CHANNELS);
    for(int c=0; c<N_CHANNELS; c++)
    {
        D_obs.submat(3, c, D_obs.n_rows - 1, c) = \
            pred_x.submat(3 * N_LAGS * c, 0, 3 * N_LAGS * (c + 1) - 1, 0);
    }

    // The double derivative of the observation vector with respect to the
    // state, evaluated at estimated_obs.
    cube DD_obs = zeros<cube>(pred_x.n_rows, pred_x.n_cols, N_CHANNELS);
    
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
    pos = new_x.submat(0, 0, 2, 0);
    uHistory = new_x.submat(3, 0, new_x.n_rows - 1, 0);
    if(covReset.compare("posOnly") == 0)
        covariance = blkdiag(initialArmCov.submat(0, 0, 2, 2), \
            new_cov.submat(3, 3, new_cov.n_rows - 1, new_cov.n_cols - 1));
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

mat ComputerEEGFilterCursorOnly::newTrial() {
    pos = randomStartLocation();
    uHistory = zeros<mat>(3, N_LAGS);
    
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
