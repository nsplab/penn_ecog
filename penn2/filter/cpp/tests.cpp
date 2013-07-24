
#include <armadillo>
#include <iostream>
#include <itpp/base/random.h>
#include "reachstateequation.h"
#include "jointrse_filter.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

using namespace std;
using namespace arma;

typedef unsigned long long uint64;

uint64 GetTimeMs64();

void testRSE() {
    ofstream rseTraject("rseTrajectory.txt");

    float originX =  4.0;
    float originY = -4.0;
    float originZ =  5.0;

    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);
    reachTarget<<originX<<endr<<originY<<endr<<originZ<<endr<<0<<endr<<0<<endr<<0<<endr;
    int reachTimeSteps = 1.0/timeBin;

    uint64 t1 = GetTimeMs64();
    for (size_t i=0; i<100; i++) {
        reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
        reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();
    }
    uint64 t2 = GetTimeMs64();
    cout<<"time: "<<(t2-t1)<<endl;

    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();

    for (size_t i=0; i<100; i++) {
        itpp::Uniform_RNG uniGen(0, 1);
        float u = uniGen();
        float v = uniGen();
        float theta = 2.0 * itpp::pi * u;
        float phi = acos(2.0*v-1.0);
        float r = 15;
        float x = r * cos(theta) * sin(phi) + originX;
        float y = r * sin(theta) * sin(phi) + originY;
        float z = r * cos(phi) + originZ;

        vec handState;
        handState<<x<<y<<z<<0<<0<<0;

        for (float time=0, timeStep=0; time<(3.0-timeBin); time+=timeBin, timeStep+=1) {
            handState = rseParams.F.slice(timeStep) * handState + rseParams.b.slice(timeStep);
            //cout<<rseParams.F.slice(timeStep)<<endl;
            rseTraject<<handState(0)<<" "<<handState(1)<<" "<<handState(2)<<" "<<handState(3)<<" "<<handState(4)<<" "<<handState(5)<<";"<<endl;
        }
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
          <<0.0<<0.0<<0.0<<-3.0<<0.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<4.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<0.0<<0.0<<2.5<<endr;

    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(6, 1);
    int reachTimeSteps = 1.0/timeBin;
    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    reachStateEquation::RSEMatrixStruct rseParams = rseComputer.returnAnswer();

    jointRSE_filter filter(3,true,false,true,true);

    for (size_t trial=1; trial<40; trial++) {
        cout<<"trial: "<<trial<<endl;
        // random point on sphere as initial hand position
        // http://mathworld.wolfram.com/SpherePointPicking.html
        itpp::Uniform_RNG uniGen(0, 1);
        float u = uniGen();
        float v = uniGen();
        float theta = 2.0 * itpp::pi * u;
        float phi = acos(2.0*v-1.0);
        float r = 15;
        float x = r * cos(theta) * sin(phi);
        float y = r * sin(theta) * sin(phi);
        float z = r * cos(phi);

        vec handState;
        handState<<x<<y<<z<<0<<0<<0;

        vector<float> handPos(3); handPos[0]=x; handPos[1]=y; handPos[2]=z;

        for (size_t step=0; step<300; step++) {
            cout<<"step: "<<step<<endl;
            handState = rseParams.F.slice(step) * handState + rseParams.b.slice(step);
            for (size_t i=0; i<handState.n_rows; i++)
                origTraject<<handState(i)<<" ";
            origTraject<<";"<<endl;
            vec noise = randn<vec>(6);
            cout<<"noise: "<<noise<<endl;
            vec noiseHandState = handState;
            for (size_t i=0; i<noiseHandState.n_rows; i++)
                noisTraject<<noiseHandState(i)<<" ";
            noisTraject<<";"<<endl;
            mat obs = obsMat * noiseHandState + noise;
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

    system("matlab -nosplash -nodesktop -r \"plot_innovation;quit;\"");
}


/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

uint64 GetTimeMs64()
{
#ifdef WIN32
 /* Windows */
 FILETIME ft;
 LARGE_INTEGER li;

 /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
  * to a LARGE_INTEGER structure. */
 GetSystemTimeAsFileTime(&ft);
 li.LowPart = ft.dwLowDateTime;
 li.HighPart = ft.dwHighDateTime;

 uint64 ret = li.QuadPart;
 ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
 ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

 return ret;
#else
 /* Linux */
 struct timeval tv;

 gettimeofday(&tv, NULL);

 uint64 ret = tv.tv_usec;
 /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
 ret /= 1000;

 /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
 ret += (tv.tv_sec * 1000);

 return ret;
#endif
}
