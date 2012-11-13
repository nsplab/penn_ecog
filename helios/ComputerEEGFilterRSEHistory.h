#ifndef COMPUTEREEGFILTERRSE_H
#define COMPUTEREEGFILTERRSE_H

#include <armadillo>
using namespace arma;

class ComputerEEGFilterRSE {
    private:
        mat channelParametersHat, pos, uHistory, covariance, \
            initialChannelCov, initialArmCov, channelVariances;
        std::string covReset;
        cube F_channel, F_arm, b_arm, Q_channel, Q_arm, F, b, Q;
        double timeBin;
        int timeStep;
    public:
        /* Basic constructor. Reads in parameters for creation from 
           parameters.cpp. */
        ComputerEEGFilterRSE();
        
        /* The map<string, mat> returned by filter has the following fields: 
               'position': A 3x1 matrix. Ordered (x, y, z).
               'velocity': A 3x1 matrix. Ordered (x, y, z).
               'channelParameters': A N_CHANNELS x 3 * N_LAGS matrix. Each row
                                    contains all the parameters from a single
                                    channel, ordered
                                        a_0, a_-1, ..., a_-(N_LAGS-1),
                                        b_0, b_-1, ..., b_-(N_LAGS-1),
                                        c_0, c_-1, ..., c_-(N_LAGS-1)
                                    where each a_-i is the parameter
                                    corresponding to the x-component of the
                                    velocity from i time steps in the past, b_-i
                                    corresponds to the y-component, and c_-i
                                    corresponds to the z-component.
               'state': A (3 * N_LAGS * N_CHANNELS + 3 + 3 * N_LAGS)x1 matrix of
                        all the channel parameters, the position, and
                        velocities from the previous N_LAGS time steps. All this
                        information is available from other fields. The first
                        elements in the state vector are the 3 * N_LAGS *
                        N_CHANNELS channel parameters, ordered first by channel
                        number, then by x/y/z directionality, and finally by
                        lag number (from 0 to -(N_LAGS - 1)). The next 3
                        elements of the state comprise the position, ordered
                        x/y/z, and the final 3 * N_LAGS elements are the history
                        of decoded velocities, ordered first by x/y/z
                        directionality and then by lag number.
               'covariance': The corresponding covariance matrix for the state
                             vector. */
        std::map<std::string, mat> filter(const vec &observations);
        
        /* Primes the ComputerEEGFilter for a new trial. Resets all aspects
           related to the kinematics, but retains channel parameter estimates.
           */
        mat newTrial(void);
        
        /* Basic accessor. */
        mat getChannelParameters();
        
        /* Basic mutator. */
        void setChannelParameters(mat newChannelParameters);
};

#endif
