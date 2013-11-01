#include <iostream>
#include <string>
#include <string.h>

#include <eigen3/Eigen/Dense>
#include <zmq.hpp>

#include "GetPot.h"

#include "naive_filter.h"
#include "jointrse_filter.h"

#include "tests.h"

int main(int argc, char** argv) {

    //NaiveFilter filter;
    // size_t dim, bool velocityParams, bool positionParams, bool affineParam, bool useRSE, bool timeInvariant, bool log , float maxTrialTime
    jointRSE_filter filter(3, true, false, true, true, false, true, 7.0);
    filter.Run();
    //filter.RunPredictOnly();

    //testRSE();

    return 0;
}
