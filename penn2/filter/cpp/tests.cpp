
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
    int reachTimeSteps = 2.0/timeBin;

    uint64 t1 = GetTimeMs64();
    for (size_t i=0; i<100; i++) {
        reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
        RSEMatrixStruct rseParams = rseComputer.returnAnswer();
    }
    uint64 t2 = GetTimeMs64();
    cout<<"time: "<<(t2-t1)<<endl;

    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget);
    RSEMatrixStruct rseParams = rseComputer.returnAnswer();

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
            mat randomNoise  = randn<vec>(6);
            for (size_t i=0; i<6; i++) {
                randomNoise(i) = randomNoise(i) * sqrt(rseParams.Q.slice(timeStep)(i,i));
            }
            handState = rseParams.F.slice(timeStep) * handState + randomNoise + rseParams.b.slice(timeStep);
            //cout<<rseParams.Q.slice(timeStep)<<endl;
            rseTraject<<handState(0)<<" "<<handState(1)<<" "<<handState(2)<<" "<<handState(3)<<" "<<handState(4)<<" "<<handState(5)<<";"<<endl;
        }
    }
}

const size_t dim = 1;

void testJointFilter() {

    ofstream origTraject("origTrajectory.txt");
    ofstream noisTraject("noisTrajectory.txt");
    ofstream filtTraject("filtTrajectory.txt");

    vector<float> target(dim);
    vector<float> features(jointRSE_filter::numChannels);
    for (int i = 0; i < dim; i++) {
        target[i] = 0;
    }

    mat obsMat;
    // velocity
    //obsMat<<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<2.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<3.0<<endr
    //      <<0.0<<0.0<<0.0<<-3.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<2.0<<4.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<1.5<<2.5<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr;
    //obsMat<<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr
    //      <<0.0<<0.0<<0.0<<1.0<<1.0<<1.0<<endr;
    //obsMat<<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<2.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<5.0<<endr
    //      <<0.0<<0.0<<0.0<<-3.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<4.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<2.5<<endr;
    // position
    //obsMat<<-3.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<4.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<2.5<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr;
    //obsMat<<1.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<1.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<1.0<<0.0<<0.0<<0.0<<endr
    //      <<1.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<1.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<1.0<<0.0<<0.0<<0.0<<endr
    //      <<1.0<<1.0<<1.0<<0.0<<0.0<<0.0<<endr;
    //obsMat<<1.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<1.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<1.0<<0.0<<0.0<<0.0<<endr;

    //obsMat<<0.0<<0.0<<0.0<<1.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<1.0<<0.0<<endr
    //      <<0.0<<0.0<<0.0<<0.0<<0.0<<1.0<<endr;
    obsMat<<0.0<<0.0<<1.0<<0.0<<endr
          <<0.0<<0.0<<0.0<<1.0<<endr;
    obsMat<<1.0<<0.0<<0.0<<0.0<<endr
          <<0.0<<1.0<<0.0<<0.0<<endr;
    //obsMat<<1.0<<0.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<1.0<<0.0<<0.0<<0.0<<0.0<<endr
    //      <<0.0<<0.0<<1.0<<0.0<<0.0<<0.0<<endr;

    //obsMat << 1.8339 << 3.5784 <<-0.2050 << 0.7172 <<-0.7873 << 0.3252 << endr
    //       <<-2.2588 << 2.7694 <<-0.1241 << 1.6302 << 0.8884 <<-0.7549 << endr
    //       << 0.8622 <<-1.3499 << 1.4897 << 0.4889 <<-1.1471 << 1.3703 << endr
    //       << 0.3188 << 3.0349 << 1.4090 << 1.0347 <<-1.0689 <<-1.7115 << endr
    //       <<-1.3077 << 0.7254 << 1.4172 << 0.7269 <<-0.8095 <<-0.1022 << endr
    //       <<-0.4336 <<-0.0631 << 0.6715 <<-0.3034 <<-2.9443 <<-0.2414 << endr
    //       << 0.3426 << 0.7147 <<-1.2075 << 0.2939 << 1.4384 << 0.3192 << endr;

    //obsMat << 0 << 0 << 0 << 0.7172 <<-0.7873 << 0.3252 << endr
    //       << 0 << 0 << 0 << 1.6302 << 0.8884 <<-0.7549 << endr
    //       << 0 << 0 << 0 << 0.4889 <<-1.1471 << 1.3703 << endr
    //       << 0 << 0 << 0 << 1.0347 <<-1.0689 <<-1.7115 << endr
    //       << 0 << 0 << 0 << 0.7269 <<-0.8095 <<-0.1022 << endr
    //       << 0 << 0 << 0 <<-0.3034 <<-2.9443 <<-0.2414 << endr
    //       << 0 << 0 <<-0 << 0.2939 << 1.4384 << 0.3192 << endr;

    //obsMat << 0.7172 <<-0.7873 << 0.3252 << 0 << 0 << 0 << endr
    //       << 1.6302 << 0.8884 <<-0.7549 << 0 << 0 << 0 << endr
    //       << 0.4889 <<-1.1471 << 1.3703 << 0 << 0 << 0 << endr
    //       << 1.0347 <<-1.0689 <<-1.7115 << 0 << 0 << 0 << endr
    //       << 0.7269 <<-0.8095 <<-0.1022 << 0 << 0 << 0 << endr
    //       <<-0.3034 <<-2.9443 <<-0.2414 << 0 << 0 << 0 << endr
    //       << 0.2939 << 1.4384 << 0.3192 << 0 << 0 <<-0 << endr;
    obsMat<<1.0<<0.0<<endr;
    obsMat<<0.0<<1.0<<endr;

    const double timeBin = 0.01;
    const double maxTimeSteps = 3.0/timeBin;
    mat reachTarget = zeros<mat>(2*dim, 1);
    int reachTimeSteps = 1.0/timeBin;
    bool timeInvariant = true;
    RSEMatrixStruct rseParams;
    if (timeInvariant) {
        const double alpha = 0.18;
        const double beta = 0.1;
        const double gamma = 0.1;
        mat Q = zeros<mat>(2 * dim, 2 * dim);
        for (size_t i = 0; i < dim; i++) {
            Q(i, i) = alpha;
            Q(dim + i, dim + i) = beta;
        }
        mat R = gamma * eye<mat>(dim, dim);

        timeInvariantRSE rseComputer(reachTarget, Q, R, dim);
        rseParams = rseComputer.returnAnswer();
    }
    else {
        reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget, dim);
        rseParams = rseComputer.returnAnswer();
    }


<<<<<<< HEAD
    for (size_t trial=1; trial<50; trial++) {
=======
    jointRSE_filter filter(dim,true,true,true,true,true,true);

    for (size_t trial=1; trial<=100; trial++) {
>>>>>>> 9ed04f93134e5b0d05255ce4584f5259824b7701
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

        if (dim == 2) {
            x = r * cos(theta);
            y = r * sin(theta);
        }
        if (dim == 1) {
            if (x > 0) {
                x = r;
            }
            else {
                x = -r;
            }
        }

        //x = 0;
        //y = 0;
        //z = 15;
        vec handState;
        if (dim == 3) {
            handState<<x<<y<<z<<0<<0<<0;
        }
        if (dim == 2) {
            handState<<x<<y<<0<<0;
        }
        if (dim == 1) {
            handState<<x<<0;
        }

        vector<float> handPos(dim);
        if (dim >= 1) {
            handPos[0]=x;
        }
        if (dim >= 2) {
            handPos[1]=y;
        }
        if (dim >= 3) {
            handPos[2]=z;
        }

        for (size_t step=0; step<300; step++) {
            cout<<"step: "<<step<<endl;
            handState = rseParams.F.slice(timeInvariant ? 0 : step) * handState + rseParams.b.slice(timeInvariant ? 0 : step);
            for (size_t i=0; i<handState.n_rows; i++)
                origTraject<<handState(i)<<" ";
            cout<<"handState: "<<handState<<endl;
            origTraject<<";"<<endl;
            vec noise = 0.0*randn<vec>(jointRSE_filter::numChannels) + 5.21;
            //vec noise = zeros<vec>(numChannels);
            cout<<"noise: "<<noise<<endl;
            vec noiseHandState = handState;
            for (size_t i=0; i<noiseHandState.n_rows; i++)
                noisTraject<<noiseHandState(i)<<" ";
            noisTraject<<";"<<endl;
            mat obs = obsMat * noiseHandState + noise;
            // pass noise
            features = conv_to< std::vector<float> >::from(obs);
            cout << "features: \n";
            for (int i = 0; i < features.size(); i++) {
              cout << features[i] << "\t";
            }
            cout << "\n";
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
