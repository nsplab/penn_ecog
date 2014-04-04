#include <iostream>
#include <string>
#include <string.h>

#include <eigen3/Eigen/Dense>
#include <zmq.hpp>

#include <signal.h>

#include "../../libs/GetPot.h"

#include "naive_filter.h"
#include "jointrse_filter.h"

#include "tests.h"

using namespace std;

bool quit = false;

void signal_callback_handler(int signum) {
    cout<<"int signal"<<endl;
    signal(signum, SIG_IGN);
    quit = true;
}

int main(int argc, char** argv) {

    string cfgFile("../filter.cfg"); //filter.cfg, in /penn_ecog_penn2/filter/cpp/, contains all relevant filter parameters for all filter types.
                                     //eg.) for adaptive filtering, it contains increment covariances, choice of state equation, etc.

    // check if the config file exists
    ifstream testFile(cfgFile.c_str());
    if (! testFile.good()) {
        cout<<"Could not open the config file: "<<cfgFile<<endl;
        return 1;
    } else {
        testFile.close();
    }

    // parse config file
    GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();

    int dimensions = ifile("dimensions", 3);
    int velocityParams = ifile("velocityParams", 1);
    int positionParams = ifile("positionParams", 0);
    int affineParam = ifile("affineParam", 1);
    int useRSE = ifile("useRSE", 1);
    int timeInvariant = ifile("timeInvariant", 0);
    int log = ifile("log", 1);
    double trialTime = ifile("trialTime", 3.0);
    double maxTrialTime = ifile("maxTrialTime", 7.0);
    double diagQ = ifile("diagQ", 1.0e-3);
    double finalPosCov = ifile("finalPosCov", 1.0e-6);
    double finalVelCov = ifile("finalVelCov", 1.0e-8);
    double channelCov = ifile("channelCov", 1.0);
    double initialArmPosVar = ifile("initialArmPosVar", 1.0);
    double initialArmVelVar = ifile("initialArmVelVar", 1.0);
    unsigned numLags = ifile("numLags", 1);
    unsigned filterType = ifile("filterType", 0);

    string featureConfig = ifile("featureConfig", "feature.cfg");
    GetPot ifileFeature(featureConfig.c_str(), "#", "\n");
    unsigned featureRate = ifileFeature("outputRate", 10);
    cout<<"featureRate: "<<featureRate<<endl;

    // check if featureConfig exists
    testFile.open(featureConfig.c_str());
    if (! testFile.good()) {
        cout<<"Could not open featureConfig: "<<featureConfig<<endl;
        return 1;
    } else {
        testFile.close();
    }


    if (filterType == 0) {
        NaiveFilter filter(featureRate);
        filter.Run();
    } else {
        jointRSE_filter filter(dimensions,     // dimension
                           velocityParams, // boolean which determines whether to include velocity parameters in the observation model
                           positionParams, // boolean which determines whether to include position parameters in the observation model
                           affineParam,    // boolean which determines whether to include affine parameters in the observation model
                           useRSE,         // boolean which indicates whether to use RSE as the state equation
                           timeInvariant,  // boolean which indicates whether to use a time invariant model as the state equation
                           log,            // records various values to file: innovations, prediction and update values at every timestep; used for debugging jointRSE_filter
                           trialTime,      // trial time
                           maxTrialTime,   // maximum trial time
                           diagQ,          // initial RSE cov diagonal value
                           finalPosCov,    // target position variance
                           finalVelCov,    // target velocity variance
                           featureRate,    // feature rate
                           channelCov,     // channel parameter variance
                           initialArmPosVar,
                           initialArmVelVar,
                           true,           // integrate velocity to get position
                           numLags
                           );
        filter.Run();
    }
    

    //filter.RunPredictOnly();

    //testRSE();
    //testJointFilter(3, filter);

    return 0;
}
