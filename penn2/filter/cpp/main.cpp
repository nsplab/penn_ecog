#include <iostream>
#include <string>
#include <string.h>

#include <eigen3/Eigen/Dense>
#include <zmq.hpp>

#include "GetPot.h"

#include "naive_filter.h"
#include "jointrse_filter.h"

#include "tests.h"

using namespace std;

int main(int argc, char** argv) {

    string cfgFile("../filter.cfg");

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
                           velocityParams, // include velocity parameters
                           positionParams, // include position parameters
                           affineParam,    // include affine parameter
                           useRSE,         // use RSE
                           timeInvariant,  // time invariant switch
                           log,            // log the variables
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
