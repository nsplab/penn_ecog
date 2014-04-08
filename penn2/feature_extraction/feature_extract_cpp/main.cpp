#include <iostream>
#include <string>
#include <string.h>
#include <iomanip>
#include <fstream>

#include <signal.h>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <zmq.hpp>

#include <chrono>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>

#include "GetPot.h"
#include "../../libs/stft/fft.h"
#include "../../libs/decimate/decimate.h"

using namespace std;
using namespace zmq;
using namespace Eigen;

typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::right> > > accumulator_t_right;
typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::left> > > accumulator_t_left;

int parsConfig(string& signalConfig, string& matrixFile, size_t& fftWinSize,
               size_t& fftWinType, size_t& outputRate, string& spatialFilterFile,
               unsigned& frqRangeFrom, unsigned& frqRangeTo, unsigned& baseline,
               size_t& numFeatureChannels);

int GetWinSizeSamples(string signalConfig, size_t& fftWinSizeSamples,
                      size_t fftWinSize, size_t& numChannels, size_t& samplingRate,
                      size_t& neuralIndex);

void loadMatrix(SparseMatrix<float>& coefMx, string matrixFile, vector<unsigned>& rows, size_t numChannels, size_t numFeatureChannels);

bool quit = false;
void signal_callback_handler(int signum) {
    cout<<"int signal"<<endl;
    signal(signum, SIG_IGN);
    quit = true;
}

int main(int argc, char** argv)
{

    time_t rawtime;
    time(&rawtime);
    char nameBuffer[40];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S.txt", ptm);
    string dataFilename = string("baselineData_")+string(nameBuffer);
    ofstream baselineDataFile(dataFilename);
    ofstream baselineDataFileCopy;


    baselineDataFile.precision(10);


    signal(SIGINT, signal_callback_handler);

    string signalConfig;
    string matrixFile;
    string spatialFilterFile;
    size_t fftWinSize;
    size_t fftWinType;
    size_t outputRate;
    unsigned frqRangeFrom;
    unsigned frqRangeTo;
    unsigned baseline;

    // the spatial matrix is numFeatureChannels by numChannels
    size_t numFeatureChannels;

    if (0 != parsConfig(signalConfig, matrixFile, fftWinSize,
                        fftWinType, outputRate, spatialFilterFile,
                        frqRangeFrom, frqRangeTo, baseline,
                        numFeatureChannels)) {
        return 1;
    }

    if (argc > 1) {
        int a1 = atoi(argv[1]);
        if (a1 == 1) {
            baseline = true;
        } else {
            baseline = false;
        }
    }


    if (baseline) {
        baselineDataFileCopy.open("baselineData.txt");
        baselineDataFileCopy.precision(10);
    }


    size_t numChannels;// = rows.size();//spatialFilterMx.rows();
    // compute fft window size in number of samples
    size_t fftWinSizeSamples;
    size_t samplingRate;
    size_t neuralIndex;
    if (0 != GetWinSizeSamples(signalConfig, fftWinSizeSamples,
                               fftWinSize, numChannels, samplingRate,
                               neuralIndex)) {
        return 1;
    }


    if (argc > 2) {
        samplingRate = atof(argv[2]);
    }

    if (argc > 3) {
        numChannels = atoi(argv[3]);
    }

    SparseMatrix<float> spatialFilterMx(numFeatureChannels, numChannels);
    vector<unsigned> rows;
    //Matrix<float, 60, 60> spatialFilterMx;
    loadMatrix(spatialFilterMx, spatialFilterFile, rows, numChannels, numFeatureChannels);

    vector<float> pwrFeature(numFeatureChannels);

    //Matrix<float, 60, 1> result;

    //SparseVector<float> sresult(60);
    //sresult = spatialFilterMx.rowwise().sum();

    cout<<"********* result"<<endl;
    //cout<<result<<endl;


    // this allows selecting channels by the spatial filter matrix, before
    // computing the FFT

    Fft<float> fft(fftWinSizeSamples, Fft<float>::windowFunc::BLACKMAN_HARRIS, samplingRate/2.0, numFeatureChannels);

    unsigned binFrom = fft.GetBin(frqRangeFrom);
    unsigned binTo = fft.GetBin(frqRangeTo);

    context_t context(3);
    socket_t publisher(context, ZMQ_PUB);
    socket_t subscriber(context, ZMQ_SUB);
    int hwm = 1;				//hwm - high water mark - determines buffer size for
    //data passed through ZMQ. hwm = 1 makes the ZMQ buffer
    //size = 1. This means that if no module has accessed a
    //value written through ZMQ, new values will be dropped
    //until any module reads the value
    publisher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    publisher.bind("ipc:///tmp/features.pipe");
    subscriber.connect("ipc:///tmp/signal.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);


    float buffer[64];
    vector<float> points(numChannels);
    vector<float> decimatedPoints(numChannels);
    vector<float> spatiallyFilteredPoints(numFeatureChannels);

    size_t numberOfSamplesSkip = samplingRate / outputRate / 2; // / 2 due to downsampling
    size_t prevProcSample = 0;

    unsigned freqRange = fftWinSizeSamples / 2 + 1;
    vector<vector<float> > powers(numFeatureChannels);
    for (unsigned i=0; i<numFeatureChannels; i++) {
        powers[i].resize(freqRange);
    }

    float alpha = 0.1;
    float movingAvg = 0.0;

    Decimate decimater(numChannels);

    MatrixXf spatialFilteredChannels(numFeatureChannels, 1);

    // if running in the baseline mode feature statistics are collected and stored in a text file
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::variance> > acc[numFeatureChannels];
    const int c = 10000;
    accumulator_t_right accRight( boost::accumulators::tag::tail<boost::accumulators::right>::cache_size = c );
    accumulator_t_left accLeft( boost::accumulators::tag::tail<boost::accumulators::left>::cache_size = c );


    ofstream dataLogFile("datalog.txt");

    size_t counter = 0;
    while (!quit) {
        message_t signal;
        try {
            subscriber.recv(&signal);
        } catch (...) {
            cout<<"zmq::recv interrupted"<<endl;
            break;
        }

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        // extract timestamp
        size_t timestamp;
        memcpy(&timestamp, signal.data(), sizeof(size_t));
        //cout<<"timestamp: "<<timestamp<<endl;

        // extract neural signal
        memcpy(buffer, (size_t*)signal.data()+1, signal.size()-sizeof(size_t));
        memcpy(points.data(), &(buffer[neuralIndex]), sizeof(float) * numChannels);

        if (decimater.AddSample(points)) {
            decimater.GetDecSample(decimatedPoints);
            //if (timestamp % 1000 == 0) {
                //cout<<"p "<<points[0]<<endl;
                //cout<<"d "<<decimatedPoints[0]<<endl;
                //cout<<"t "<<timestamp<<endl;
                counter ++;
            //}

            /// TODO: FIXME
            // baseline assume first feature is the one we want

            MatrixXf decVec(numChannels,1);
            decVec = Map<MatrixXf>(decimatedPoints.data(), numChannels, 1);

            spatialFilteredChannels = spatialFilterMx * decVec;


            memcpy(spatiallyFilteredPoints.data(), spatialFilteredChannels.data(), sizeof(float) * numFeatureChannels);
            //for (unsigned i=0; i<numFeatureChannels; i++) {
            //    spatiallyFilteredPoints[i] = spatialFilteredChannels(i);
            //}

            fft.AddPoints(spatiallyFilteredPoints);

            if (counter % (numberOfSamplesSkip) == 0)
            if (fft.Process()) {
                fft.GetPower(powers);

                for (unsigned ch=0; ch<numFeatureChannels; ch++) {
                    float pwr = 0.0;
                    cout<<binFrom<<" "<<binTo<<endl;
                    cout<<frqRangeFrom<<" "<<frqRangeTo<<endl;
                    for (unsigned bin=binFrom; bin<=binTo; bin++) {
                        pwr += powers[ch][bin];
                        //cout<<"ch: "<<ch<<"  bin:"<<bin<<" pwr:"<<powers[ch][bin]<<endl;
                    }
                    pwrFeature[ch] = pwr;
                    if (baseline == 1) {
                        acc[ch](pwr);
                    }
                    cout<<"ch: "<<ch<<" pwr:"<<pwr<<"\t";
                }

                if (baseline == 1) {
                    baselineDataFile<<pwrFeature[0]<<endl;
                    baselineDataFileCopy<<pwrFeature[0]<<endl;
                }
                cout<<endl;

                dataLogFile<<pwrFeature[0]<<endl;

                // should add the timestamp to the feature message
                message_t zmq_message(numFeatureChannels * sizeof(float) + sizeof(size_t));
                memcpy(zmq_message.data(), &timestamp, sizeof(size_t));
                memcpy(static_cast<size_t*>(zmq_message.data())+1, pwrFeature.data(), zmq_message.size());
                publisher.send(zmq_message);

                end = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_seconds = end-start;

                //cout<<"t "<<timestamp<<endl;
                //cout<<"c "<<counter<<endl;
                cout<< "elapsed time: " << elapsed_seconds.count() << "s\n";
            }
        }

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


        //fft.AddPoints(points);
        //if ((prevProcSample + numberOfSamplesSkip) > timestamp){
        //    continue;
        //}
        /*if (timestamp % (24400/10) == 0)
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

            /*float avgPower = 0.0;
            for (unsigned i=0; i<10; i++) {
                for (unsigned j=36;j<46;j++) {
                    avgPower += powers[i][j];
                }
            }

            movingAvg = alpha * avgPower + (1.0 - alpha) * movingAvg;

            cout<<"movingAvg: "<<movingAvg<<endl;

            stringstream sendMsg;
            sendMsg<<timestamp<<" "<<(movingAvg*500.0-15.0);

            message_t zmq_message(sendMsg.str().length());
            memcpy((char *) zmq_message.data(), sendMsg.str().c_str(), sendMsg.str().length());
            publisher.send(zmq_message);

            message_t supervisor_msg;
            publisher.recv(&supervisor_msg);
            */

            //message_t featuesMsg(sizeof(size_t)+features.rows()*sizeof(float));
            //memcpy(featuesMsg.data(), &timestamp, sizeof(size_t));
            //memcpy(static_cast<size_t*>(featuesMsg.data())+1, features.data(), features.rows()*sizeof(float));
            //publisher.send(featuesMsg);
        //}
    }
    dataLogFile.close();

    cout<<"stats"<<endl;
    if (baseline == 1) {

        time_t rawtime;
        cout<<boost::accumulators::mean(acc[0])<<endl;
        cout<<sqrt(boost::accumulators::variance(acc[0]))<<endl;
        time(&rawtime);
        char nameBuffer[40];
        tm * ptm = localtime(&rawtime);
        strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S.txt", ptm);
        string dataFilename = string("baseline_")+string(nameBuffer);
        ofstream baselineFile(dataFilename);
        ofstream baselineFileCopy("baseline.txt");

        baselineFile.precision(10);
        baselineFileCopy.precision(10);

        for (unsigned ch=0; ch<numFeatureChannels; ch++) {
            baselineFile<<boost::accumulators::mean(acc[ch])<<" ";
            baselineFile<<boost::accumulators::variance(acc[ch])<<endl;
            baselineFileCopy<<boost::accumulators::mean(acc[ch])<<" ";
            baselineFileCopy<<boost::accumulators::variance(acc[ch])<<endl;
        }
        cout<<"files are written"<<endl;
    }

    return 0;
}

int parsConfig(string& signalConfig, string& matrixFile, size_t& fftWinSize,
               size_t& fftWinType, size_t& outputRate, string& spatialFilterFile,
               unsigned& frqRangeFrom, unsigned& frqRangeTo, unsigned& baseline,
               size_t& numFeatureChannels) {

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

    spatialFilterFile = ifile("spatialFilterFile", "");
    //spatialFilterFile = "featuremx.csv";
    /*// check if spatialFilterFile exists
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
    frqRangeFrom = ifile("freqRangeFrom", 75);
    frqRangeTo = ifile("freqRangeTo", 100);
    baseline = ifile("baseline", 0);

    numFeatureChannels = ifile("numFeatureChannels", 60);

    return 0;
}


int GetWinSizeSamples(string signalConfig, size_t& fftWinSizeSamples,
                      size_t fftWinSize, size_t& numChannels, size_t& samplingRate,
                      size_t& neuralIndex) {

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

    neuralIndex = ifile("neuralIndex", 0);

    // outSampleRate/2.0: because the signal gets downsampled by a factor of 2
    // in the main function
    fftWinSizeSamples = fftWinSize / 1000.0 * (outSampleRate/2.0);

    cout<<"fftWinSizeSamples "<<fftWinSizeSamples<<endl;

    return 0;
}

void loadMatrix(SparseMatrix<float>& coefMx, string matrixFile, vector<unsigned>& rows, size_t numChannels, size_t numFeatureChannels) {
    ifstream mxFile(matrixFile.c_str());

    typedef Eigen::Triplet<float> T;
    std::vector<T> tripletList;
    tripletList.reserve(numChannels);

    float element = 0.0;
    size_t col=0, row=0;
    bool blankRow = true;
    while(true) {
        mxFile>>element;
        if (!mxFile.good())
            break;

        if (element != 0) {
            tripletList.push_back(T(row,col,element));
            blankRow = false;
        }
        col++;
        if (col == numChannels) {
            col = 0;
            if (!blankRow) {
                rows.push_back(row);
            }
            row++;
        }
    }

    coefMx.setFromTriplets(tripletList.begin(), tripletList.end());

    cout<<"coefMx:"<<endl;
    cout<<coefMx<<endl;
}

