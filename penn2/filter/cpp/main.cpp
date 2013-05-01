#include <iostream>
#include "naive_filter.h"
#include "reachstateequation.h"

using namespace std;
using namespace arma;

void testRSE();

int main()
{
    //cout << "Filter is running" << endl;

    //NaiveFilter naive_filter;
    //naive_filter.Run();

    testRSE();

    return 0;
}

void testRSE() {
    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);
    int reachTimeSteps = 1.0/timeBin;

    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();

    vec handState;
    handState<<10<<8<<6<<0<<0<<0;

    int timeStep = 0;
    for (float time=0, timeStep=0; time<3.0; time+=timeBin, timeStep+=1) {
        handState = rseParams.F.slice(timeStep) * handState + rseParams.b.slice(timeStep);
        cout<<rseParams.F.slice(timeStep)<<endl;
        //cout<<handState(0)<<" "<<handState(1)<<" "<<handState(2)<<";"<<endl;
    }
}

