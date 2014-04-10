#include <iostream>
#include <string>
#include <string.h>

#include <eigen3/Eigen/Dense>
#include <zmq.hpp>

#include <signal.h>

#include "../../libs/GetPot.h"  //this open-source library let's us parse text from config files to easily store and retrieve filter parameters.

#include "moving_average_filter.h"
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

                                                                // check if the filter.cfg config file exists
    ifstream testFile(cfgFile.c_str());                         // declare an object of the class ifstream, which ties a file to the program for input (no file writing)
    if (! testFile.good()) {                                    // if the filter config file does not exist
        cout<<"Could not open the config file: "<<cfgFile<<endl;// print an error message to the terminal
        return 1;                                               // end the program gracefully
    } else {                                                    // else the filter config file exists
        testFile.close();                                       // close the file
    }

    // parse config file
    GetPot ifile(cfgFile.c_str(), "#", "\n");                   // define the GetPot class ifile: initiates parsing for the config file; GetPot is an open-source utility for parsing a configuration file
    ifile.print();                                              // ifile("string_to_look_for", default value) is syntax
                                                                // loading values of these variables from 'filter.cfg' using the GetPot class instance 'ifile'.
    int dimensions = ifile("dimensions", 3);                    // workspace dimensions (1,2, or 3). search for the string "dimensions" in 'filter.cfg' and retrieve the value that it equals. If there is no such string, use the default value of 3
    int velocityParams = ifile("velocityParams", 1);            // boolean variable determines if veloctity parameters will be used in the observation model
    int positionParams = ifile("positionParams", 0);            // boolean variable determines if position parameters will be used in the observation model
    int affineParam = ifile("affineParam", 1);                  // boolean variable determines if affine parameters will be used in the observation model

                                                                // general filter parameter choices
    unsigned filterType = ifile("filterType", 0);               // choosing the filter type. static = 0; adaptive = 1 (static means the ad hoc method used by mosalam; adaptive is Kevin's jointRSE code)
    int log = ifile("log", 1);                                  // boolean variable determines whether debugging data will be logged for every time step, including innovation, prediction value, and update value (relevant to jointrse_filter.cpp)
    double maxTrialTime = ifile("maxTrialTime", 7.0);           // (in seconds) determines maximum time allowed for the user to complete the trial. (relevant to jointrse_filter.cpp)
                                                                // choosing the state equation
    int useRSE = ifile("useRSE", 1);                            // boolean variable determines whether the state equation used will be the RSE (relevant to jointrse_filter.cpp)
    int timeInvariant = ifile("timeInvariant", 0);              // boolean variable determines whether the state equation used will be a time invariant equation (relevant to jointrse_filter.cpp)

                                                                // parameters specific to jointRSE
    double trialTime = ifile("trialTime", 3.0);                 // (in seconds) determines arrival time assumed by the reach state equation. (relevant to jointrse_filter.cpp)
    double diagQ = ifile("diagQ", 1.0e-3);                      // increment covariance (duplicated on the diagonal (?)) of the random walk on which the RSE is based
    double finalPosCov = ifile("finalPosCov", 1.0e-6);          // covariance of the final position (used by the RSE; relevant to jointrse_filter.cpp)
    double finalVelCov = ifile("finalVelCov", 1.0e-8);          // covariance of the final velocity (used by the RSE; relevant to jointrse_filter.cpp)
    double channelCov = ifile("channelCov", 1.0);               // ?? not sure... related to noise in the observation model?
    double initialArmPosVar = ifile("initialArmPosVar", 1.0);   // covariance of the initial position (used by the RSE; relevant to jointrse_filter.cpp)
    double initialArmVelVar = ifile("initialArmVelVar", 1.0);   // covariance of the initial velocity (used by the RSE; relevant to jointrse_filter.cpp)
    unsigned numLags = ifile("numLags", 1);                     // ?? not sure... relates to length of history dependence in the observation model


    string featureConfig = ifile("featureConfig",               // get the filename of the *feature* configuration file from the filter.cfg file
                                 "../../../feature_extraction/"
                                 "feature_extract_cpp/"
                                 "config.cfg");
    GetPot ifileFeature(featureConfig.c_str(), "#", "\n");      // open a GetPot class ifileFeature: initiates parsing of the feature config file.
    unsigned featureRate = ifileFeature("outputRate", 10);      // retrieve the rate at which the features (typically, power in various frequency bands) are being updated.
    cout<<"featureRate: "<<featureRate<<endl;                   // print the feature reate to the console

                                                                // check if the feature configuration file exists by trying to open it
    testFile.open(featureConfig.c_str());                       // the testFile object is in the ifstream class, declared above
    if (! testFile.good()) {
        cout<<"Could not open "                                 // if the feature config file doesn't exist, print an error message to the console
              "featureConfig: "<<featureConfig<<endl;
        return 1;                                               // and quit
    } else {                                                    // otherwise
        testFile.close();                                       // close the feature config file and proceed
    }


    // create a pointer to the base filter class
    // using dynamic binding we can switch between filter at runtime
    FilterClass* filterObj;    

    // check what filter type has been requested
    switch (filterType) {
    case 0:
        filterObj = new MovingAverageFilter(featureRate);
        break;
    case 1:
    default:
        filterObj = new jointRSE_filter(dimensions,                      // dimension
                                        velocityParams,                      // boolean which determines whether to include velocity parameters in the observation model
                                        positionParams,                      // boolean which determines whether to include position parameters in the observation model
                                        affineParam,                         // boolean which determines whether to include affine parameters in the observation model
                                        useRSE,                              // boolean which indicates whether to use RSE as the state equation
                                        timeInvariant,                       // boolean which indicates whether to use a time invariant model as the state equation
                                        log,                                 // records various values to file: innovations, prediction and update values at every timestep; used for debugging jointRSE_filter
                                        trialTime,                           // assumed duration of the reach (sec) used by the RSE
                                        maxTrialTime,                        // maximum time (sec) allowed to the user before trial expires
                                        diagQ,                               // increment covariance (duplicated on the diagonal (?)) of the random walk on which the RSE is based
                                        finalPosCov,                         // covariance of the final position (used by the RSE; relevant to jointrse_filter.cpp)
                                        finalVelCov,                         // covariance of the final velocity (used by the RSE; relevant to jointrse_filter.cpp)
                                        featureRate,                         // rate at which the features (typically, power in various frequency bands) are being updated
                                        channelCov,                          // ?? not sure... related to noise in the observation model?
                                        initialArmPosVar,                    // covariance of the initial position (used by the RSE; relevant to jointrse_filter.cpp)
                                        initialArmVelVar,                    // covariance of the initial velocity (used by the RSE; relevant to jointrse_filter.cpp)
                                        true,                                // ?? not sure... relates to length of history dependence in the observation model
                                        numLags);
        break;
    }


    // run the main loop of the selected function
    filterObj->Run();                                           // run the filter
    
    // delete the create object on heap
    delete filterObj;

    //filter.RunPredictOnly();                                  // debugging code
    //testRSE();                                                // debugging code
    //testJointFilter(3, filter);                               // debugging code

    return 0;                                                  // end the program if it reaches this point (which it shouldn't if one of the filters are running)
}
