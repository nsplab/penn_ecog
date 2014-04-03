#include <armadillo>
#include "parameters.h"

using namespace arma;

mat generateChannelParameters() {
    mat channelParameters = zeros<mat>(N_CHANNELS, 3 * N_LAGS);
    
    return channelParameters;
}
