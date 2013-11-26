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
               size_t& fftWinType, size_t& outputRate, string& spatialFilterFile);

int GetWinSizeSamples(string signalConfig, size_t& fftWinSizeSamples,
                      size_t fftWinSize, size_t& numChannels, size_t& samplingRate);

void loadMatrix(MatrixXf& coefMx, string matrixFile);

int main(int argc, char** argv)
{

    string signalConfig;
    string matrixFile;
    string spatialFilterFile;
    size_t fftWinSize;
    size_t fftWinType;
    size_t outputRate;

    if (0 != parsConfig(signalConfig, matrixFile, fftWinSize,
                        fftWinType, outputRate, spatialFilterFile)) {
        return 1;
    }

    //MatrixXf coefMx;
    //loadMatrix(coefMx, matrixFile);

    //MatrixXf spatialFilterMx;
    //loadMatrix(spatialFilterMx, spatialFilterFile);


    //VectorXf powers;

    // this allows selecting channels by the spatial filter matrix, before
    // computing the FFT
    size_t numChannels = 10;//spatialFilterMx.rows();
    // compute fft window size in number of samples
    size_t fftWinSizeSamples;
    size_t samplingRate;
    if (0 != GetWinSizeSamples(signalConfig, fftWinSizeSamples,
                               fftWinSize, numChannels, samplingRate)) {
        return 1;
    }

    Fft<float> fft(fftWinSizeSamples, Fft<float>::windowFunc::BLACKMAN_HARRIS, samplingRate, numChannels);

    context_t context(2);
    socket_t publisher(context, ZMQ_REQ);
    socket_t subscriber(context, ZMQ_SUB);
    publisher.bind("ipc:///tmp/supervisor.pipe");
    subscriber.connect("ipc:///tmp/signal.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    float buffer[64];
    vector<float> points(numChannels);

    size_t numberOfSamplesSkip = samplingRate / outputRate;
    size_t prevProcSample = 0;

    bool quit = false;

    unsigned freqRange = fftWinSizeSamples / 2 + 1;
    vector<vector<float> > powers(numChannels);
    for (unsigned i=0; i<numChannels; i++){
        powers[i].resize(freqRange);
    }

    float alpha = 0.1;
    float movingAvg = 0.0;

    while (!quit) {
        message_t signal;
        subscriber.recv(&signal);

        // extract timestamp
        size_t timestamp;
        memcpy(&timestamp, signal.data(), sizeof(size_t));

        // extract neural signal
        memcpy(buffer, (size_t*)signal.data()+1, signal.size()-sizeof(size_t));

        // spatial filter
        //VectorXf signalVector;
        //signalVector.resize(numChannels);
        //memcpy(signalVector.data(), buffer, signal.size()-sizeof(size_t));
        //VectorXf signalSpatialFiltered = spatialFilterMx * signalVector;
        //memcpy(buffer, signalSpatialFiltered.data(), signalSpatialFiltered.rows()*sizeof(float));

        //points.resize(signalSpatialFiltered.rows());

        // compute FFT
        //copy(&(buffer[0]), &(buffer[points.size()]), points.begin());
        //for (size_t i=0; i<numChannels; i++)
        //    cout<<"p "<<i<<" : "<< points[i]<<endl;

        memcpy(points.data(), &(buffer[4]), sizeof(float) * numChannels);

        fft.AddPoints(points);
        if ((prevProcSample + numberOfSamplesSkip) > timestamp){
            continue;
        }
        if (timestamp % (24400/10) == 0)
        if (fft.Process()) {
            prevProcSample = timestamp;
            //fft.GetPowerOneVec(powers);
            //VectorXf features = coefMx * powers;
            fft.GetPower(powers);

            //cout<<"timestamp: "<<timestamp<<endl;
            //cout<<"message size: "<<(signal.size()-sizeof(size_t))/sizeof(float)<<endl;
            //cout<<"features: "<<features<<endl;


            /*if (isnan(features(0))) {
                cout<<"NaN"<<endl;

                cout<<"features "<<features<<endl;

                cout<<"points "<<endl;
                for (size_t tt=0; tt<points.size(); tt++) {
                    cout<<points[tt]<<" * ";
                }

                vector<boost::circular_buffer<float> > *buffer = fft.GetBuffer();
                for (size_t tt=0; tt<buffer->size(); tt++) {
                    cout<<"vec "<<tt<<endl;
                    for (boost::circular_buffer<float>::iterator it = (*buffer)[tt].begin(); it!= (*buffer)[tt].end(); ++it)
                        cout<<*it<<endl;
                }


                break;
            }*/

            float avgPower = 0.0;
            for (unsigned i=0; i<10; i++) {
                for (unsigned j=36;j<46;j++) {
                    avgPower += powers[i][j];
                }
            }

            movingAvg = alpha * avgPower + (1.0 - alpha) * movingAvg;

            cout<<"movingAvg: "<<movingAvg<<endl;

            stringstream sendMsg;
            sendMsg<<timestamp<<" "<<movingAvg;

            message_t zmq_message(sendMsg.str().length());
            memcpy((char *) zmq_message.data(), sendMsg.str().c_str(), sendMsg.str().length());
            publisher.send(zmq_message);

            message_t supervisor_msg;
            //publisher.recv(&supervisor_msg);


            //message_t featuesMsg(sizeof(size_t)+features.rows()*sizeof(float));
            //memcpy(featuesMsg.data(), &timestamp, sizeof(size_t));
            //memcpy(static_cast<size_t*>(featuesMsg.data())+1, features.data(), features.rows()*sizeof(float));
            //publisher.send(featuesMsg);
        }
    }

    return 0;
}

int parsConfig(string& signalConfig, string& matrixFile, size_t& fftWinSize,
               size_t& fftWinType, size_t& outputRate, string& spatialFilterFile) {

    string cfgFile("../config.cfg");

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

    /*spatialFilterFile = ifile("spatialFilterFile", "");
    // check if spatialFilterFile exists
    testFile.open(spatialFilterFile.c_str());
    if (! testFile.good()) {
        cout<<"Could not open spatialFilterFile: "<<spatialFilterFile<<endl;
        return 1;
    } else {
        testFile.close();
    }*/

    fftWinSize = ifile("fftWinSize", 500);
    fftWinType = ifile("fftWinSize", 0);
    outputRate = ifile("outputRate", 10);

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
    size_t col=0, row=0;
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

