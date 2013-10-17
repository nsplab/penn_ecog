#include <iostream>
#include <string>
#include <string.h>

#include <eigen3/Eigen/Dense>
#include <zmq.hpp>

#include "GetPot.h"
#include "fft.h"

using namespace std;
using namespace zmq;
using namespace Eigen;

int parsConfig(string& signalConfig, string& matrixFile, size_t& fftWinSize,
               size_t& fftWinType, size_t& outputRate);

int GetWinSizeSamples(string signalConfig, size_t& fftWinSizeSamples,
                      size_t fftWinSize, size_t& numChannels, size_t& samplingRate);

void loadMatrix(MatrixXf& coefMx, string matrixFile);

int main(int argc, char** argv)
{

    string signalConfig;
    string matrixFile;
    size_t fftWinSize;
    size_t fftWinType;
    size_t outputRate;

    if (0 != parsConfig(signalConfig, matrixFile, fftWinSize,
                        fftWinType, outputRate)) {
        return 1;
    }

    MatrixXf coefMx;
    loadMatrix(coefMx, matrixFile);
    VectorXf powers;

    // compute fft window size in number of samples
    size_t fftWinSizeSamples;
    size_t numChannels;
    size_t samplingRate;
    if (0 != GetWinSizeSamples(signalConfig, fftWinSizeSamples,
                               fftWinSize, numChannels, samplingRate)) {
        return 1;
    }

    Fft<float> fft(fftWinSizeSamples, Fft<float>::windowFunc(int(fftWinType)), samplingRate, numChannels);

    context_t context(2);
    socket_t publisher(context, ZMQ_PUB);
    socket_t subscriber(context, ZMQ_SUB);
    publisher.bind("ipc:///tmp/features.pipe");
    subscriber.connect("ipc:///tmp/signal.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    float buffer[numChannels];
    vector<float> points(numChannels);

    bool quit = false;
    while (!quit) {
        message_t signal;
        subscriber.recv(&signal);

        size_t timestamp;
        memcpy(&timestamp, signal.data(), sizeof(size_t));

        cout<<"timestamp: "<<timestamp<<endl;

        // skip timestamp, copy the rest
        memcpy(buffer, (size_t*)signal.data()+1, signal.size()-sizeof(size_t));
        copy(&(buffer[0]), &(buffer[numChannels-1]), points.begin());
        for (size_t i=0; i<numChannels; i++)
            cout<<"p "<<i<<" : "<< points[i]<<endl;

        fft.AddPoints(points);
        if (fft.Process()) {
            fft.GetPowerOneVec(powers);

            VectorXf features = coefMx * powers;
            //cout<<"features: "<<features<<endl;
        }
    }

    return 0;
}

int parsConfig(string& signalConfig, string& matrixFile, size_t& fftWinSize,
               size_t& fftWinType, size_t& outputRate) {

    string cfgFile("../config.cfg");

    // check if config exists
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

    signalConfig = ifile("signalConfig", "");

    // check if signalConfig exists
    testFile.open(signalConfig.c_str());
    if (! testFile.good()) {
        cout<<"Could not open signalConfig: "<<signalConfig<<endl;
        return 1;
    } else {
        testFile.close();
    }

    matrixFile = ifile("matrixFile", "");
    // check if matrixFile exists
    testFile.open(matrixFile.c_str());
    if (! testFile.good()) {
        cout<<"Could not open matrixFile: "<<matrixFile<<endl;
        return 1;
    } else {
        testFile.close();
    }

    fftWinSize = ifile("fftWinSize", 300);
    fftWinType = ifile("fftWinSize", 0);
    outputRate = ifile("outputRate", 3);

    return 0;
}


int GetWinSizeSamples(string signalConfig, size_t& fftWinSizeSamples,
                      size_t fftWinSize, size_t& numChannels, size_t& samplingRate) {

    // check if signalConfig exists
    ifstream testFile(signalConfig.c_str());
    if (! testFile.good()) {
        cout<<"Could not open the config file: "<<signalConfig<<endl;
        return 1;
    } else {
        testFile.close();
    }

    GetPot ifile(signalConfig.c_str(), "#", "\n");
    size_t outSampleRate = ifile("outSampleRate", 0);
    samplingRate = outSampleRate;
    numChannels = ifile("numChannels", 0);

    fftWinSizeSamples = fftWinSize / 1000.0 * outSampleRate;

    cout<<"fftWinSizeSamples "<<fftWinSizeSamples<<endl;

    return 0;
}

void loadMatrix(MatrixXf& coefMx, string matrixFile) {
    ifstream mxFile(matrixFile.c_str());

    size_t rows,cols;
    mxFile>>rows;
    mxFile>>cols;

    coefMx.resize(rows,cols);

    float element = 0.0;
    size_t col=0,row=0;
    while(true) {
        mxFile>>element;
        if (!mxFile.good())
            break;

        coefMx(row,col) = element;
        col++;
        if (col == cols) {
            col = 0;
            row++;
        }
    }

    cout<<"coefMx:"<<endl;
    cout<<coefMx<<endl;
}

