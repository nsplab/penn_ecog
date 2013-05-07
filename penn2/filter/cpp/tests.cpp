
#include <armadillo>
#include <iostream>
#include "reachstateequation.h"
#include "jointrse_filter.h"

using namespace std;
using namespace arma;

void testRSE() {
    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);
    int reachTimeSteps = 1.0/timeBin;

    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();

    vec handState;
    handState<<10<<8<<6<<0<<0<<0;

    for (float time=0, timeStep=0; time<3.0; time+=timeBin, timeStep+=1) {
        handState = rseParams.F.slice(timeStep) * handState + rseParams.b.slice(timeStep);
        cout<<rseParams.F.slice(timeStep)<<endl;
        //cout<<handState(0)<<" "<<handState(1)<<" "<<handState(2)<<";"<<endl;
    }
}

void testJointFilter() {

    ofstream origTraject("origTrajectory.txt");
    ofstream noisTraject("noisTrajectory.txt");
    ofstream filtTraject("filtTrajectory.txt");

    vector<float> target(3);
    vector<float> features(6);
    target[0] = 0; target[1] = 0; target[2] = 0;

    mat obsMat;
    obsMat<<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr;

    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);
    int reachTimeSteps = 1.0/timeBin;
    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();

    jointRSE_filter filter;

    for (size_t trial=1; trial<10; trial++) {
        cout<<"trial: "<<trial<<endl;
        vec handState;
        handState<<10<<8<<6<<0<<0<<0;
        vector<float> handPos(3); handPos[0]=10; handPos[1]=8; handPos[2]=6;

        for (size_t step=0; step<300; step++) {
            cout<<"step: "<<step<<endl;
            handState = rseParams.F.slice(step) * handState + rseParams.b.slice(step);
            for (size_t i=0; i<handState.n_rows; i++)
                origTraject<<handState(i)<<" ";
            origTraject<<";"<<endl;
            vec noise = randn<vec>(6);
            cout<<"noise: "<<noise<<endl;
            vec noiseHandState = handState + noise;
            for (size_t i=0; i<noiseHandState.n_rows; i++)
                noisTraject<<noiseHandState(i)<<" ";
            noisTraject<<";"<<endl;
            mat obs = obsMat * noiseHandState;
            // pass noise
            features = conv_to< std::vector<float> >::from(obs);
            //features = conv_to< std::vector<float> >::from(noise);
            filter.Simulate(features, trial, target, handPos);
            filter.Run();
            vector<float> filterHandState = filter.GetHandState();
            for (size_t i=0; i<handState.n_rows; i++)
                filtTraject<<filterHandState[i]<<" ";
            filtTraject<<";"<<endl;
        }
    }
}
