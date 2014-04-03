#ifndef COMPUTEREEGFILTERCURSORONLY_H
#define COMPUTEREEGFILTERCURSORONLY_H

#include <armadillo>
using namespace arma;

class ComputerEEGFilterCursorOnly {
    private:
        mat channelParametersHat, pos, uHistory, covariance, \
            initialArmCov, channelVariances;
        std::string covReset;
        cube F, b, Q;
        double timeBin;
        int timeStep;
    public:
        /* Basic constructor. Reads in parameters for creation from 
           parameters.cpp. */
        ComputerEEGFilterCursorOnly();
        
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
               'state': A (3 + 3 * N_LAGS)x1 matrix of the current position and
                        the velocities from the previous N_LAGS time steps. All
                        this information is available from other fields. The
                        frist elements in the state vector are the 3 elements
                        of the state that comprise the position, ordered
                        x/y/z, and the final 3 * N_LAGS elements are the history
                        of decoded velocities, ordered first by x/y/z
                        directionality and then by lag number.
               'covariance': The corresponding covariance matrix for the state
                             vector. */
        std::map<std::string, mat> filter(vec const &obs);
        
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
