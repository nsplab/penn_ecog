
#include <armadillo>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "ComputerEEGFilterRSE.h"
#include "ComputerEEGFilterCursorOnly.h"
#include "time.h"
#include "parameters.h"
#include "randomStartLocation.h"

using namespace arma;

ComputerEEGFilterRSE *trainComputer;
ComputerEEGFilterCursorOnly *testComputer;
int trialNum = -1;
int trainTrialNum = -1;
int testTrialNum = -1;
int timeStep = -2;
int holdCount = 0;
mat pos = randomStartLocation();

void InitFilter() {
    // Initialize the filters.
    trainComputer = new ComputerEEGFilterRSE();
    testComputer  = new ComputerEEGFilterCursorOnly();

    cout<<"filters initialized"<<endl;
}

void realtimeSaveForGraphing(std::string fileName, vec v)
{
    std::ofstream file(fileName.c_str(), ios::trunc);
    file<<"(";
    for(size_t i = 0; i < v.n_elem - 1; i++)
        file<<v(i)<<",";
    file<<v(v.n_elem - 1)<<")";
    for(int j = 0; j < 100; j++)
        file<<" ";
    file.flush();
    file.close();
}


/* Creates and adds to a map of matrices that maintain trial data in a way
   that is easy to graph. The list of data matrices is as follows:
       "channelParametersCube": A MAX*/
void addToGraphingMap(std::map<std::string, cube> &graphingMap, \
                      std::map<std::string, mat> dataMap)
{
    int nTrials;
    int currentTrial;

    // This allows us to not have to duplicate code for test and training
    // trials.
    if(TEST_TRIALS[trialNum])
    {
        nTrials = N_TEST_TRIALS;
        currentTrial = testTrialNum;
    }
    else
    {
        nTrials = N_TRAIN_TRIALS;
        currentTrial = trainTrialNum;
    }

    // Acquire matrices from dataMap.
    mat channelParameters = dataMap["channelParameters"];
    mat innovation = dataMap["innovation"];

    // Add information to channelParametersCube, or create it if it hasn't been
    // created yet.
    if(graphingMap.count("channelParametersCube") == 0)
    {
        cube channelParametersCube = \
                zeros<cube>(MAX_TIME_STEPS, 3 * N_LAGS, nTrials);
        mat trialSlice = channelParametersCube.slice(currentTrial);
        trialSlice.row(timeStep) = channelParameters.row(DISPLAYED_CHANNEL_NUM);
        graphingMap["channelParametersCube"] = channelParametersCube;

        realtimeSaveForGraphing("channelParameters.txt", \
            vec(channelParameters.row(DISPLAYED_CHANNEL_NUM).t()));
    }
    else
    {
        mat trialSlice = \
                graphingMap["channelParametersCube"].slice(currentTrial);
        trialSlice.row(timeStep) = channelParameters.row(DISPLAYED_CHANNEL_NUM);

        realtimeSaveForGraphing("channelParameters.txt", \
            vec(channelParameters.row(DISPLAYED_CHANNEL_NUM).t()));
    }

    // Add information to meanInnovationCube, or create it if it hasn't been
    // created yet.
    if(graphingMap.count("meanInnovationCube") == 0)
    {
        cube meanInnovationCube = zeros<cube>(MAX_TIME_STEPS, 1, nTrials);
        mat trialSlice = meanInnovationCube.slice(currentTrial);
        trialSlice.row(timeStep) = mean(abs(innovation));
        graphingMap["meanInnovationCube"] = meanInnovationCube;

        realtimeSaveForGraphing("innovations.txt", \
                                vec(trialSlice.row(timeStep)));
    }
    else
    {
        mat trialSlice = \
                graphingMap["meanInnovationCube"].slice(currentTrial);
        trialSlice.row(timeStep) = mean(abs(innovation));

        realtimeSaveForGraphing("innovations.txt", \
                                vec(trialSlice.row(timeStep)));
    }
}

/* Advances the training run to the next time step, updating the graphingMap
   with new information and putting the new value of the position of the system
   in position. The value placed in eegInstruction is
       -1 if the training run has terminated,
        0 if the training run should continue without pausing, and
        a positive integer if the training run should pause for that many
                              seconds.
   */
void nextTimeStep(vec &position, int &eegInstruction, \
                  std::map<std::string, cube> &graphingMap, \
                  vec const &eegSignals)
{
    cout<<"trialNum: "<< trialNum <<", timeStep: "<< timeStep <<endl;

    if(trialNum >= N_TRIALS)
    {
        // End the training run.
        eegInstruction = -1;
    }
    else if(timeStep == -2 || timeStep >= MAX_TIME_STEPS || \
            holdCount >= HOLD_TIME_STEPS)
    {
        // Reset timeStep and holdCount, and give main() the new position and
        // an instruction to pause. Since timeStep is set to -1, flow of
        // control will fall to the next else if block the next time
        // nextTimeStep() is called. Starting a new trial is done in this funky
        // fashion to allow main() to pause while displaying the next start
        // location.
        timeStep = -1;
        holdCount = 0;
        pos = randomStartLocation();

        position = vec(pos);
        eegInstruction = TIME_BETWEEN_TRIALS;
    }
    else if(timeStep == -1)
    {
        // Prepare the appropriate filter for a new trial.
        trialNum++;
        timeStep++;

        if(TEST_TRIALS[trialNum] == false)
        {
            trainTrialNum++;
            trainComputer->newTrial(pos);
        }
        else
        {
            testTrialNum++;
            testComputer->setChannelParameters( \
                        trainComputer->getChannelParameters());
            testComputer->newTrial(pos);
        }

        eegInstruction = 0;
        position = vec(pos);

        cout<<"Successfully completed trial initialization"<<endl;
    }
    else
    {
        // Filter the received signals, and advance the time step in the normal
        // fashion.
        std::map<std::string, mat> dataMap;
        mat pos;
        if(TEST_TRIALS[trialNum] == false)
        {
            dataMap = \
                    trainComputer->filter(eegSignals.subvec(0, N_CHANNELS - 1));
            pos = dataMap["position"];
        }
        else
        {
            dataMap = \
                    testComputer->filter(eegSignals.subvec(0, N_CHANNELS - 1));
            pos = dataMap["position"];
        }


        if(norm(REACH_TARGET_POS - pos, 2) < TARGET_RADIUS)
        {
            holdCount++;
        }
        else
        {
            holdCount = 0;
        }

        position = vec(pos);
        eegInstruction = 0; // Proceed without pausing.
        addToGraphingMap(graphingMap, dataMap);
        timeStep++;
    }
}
