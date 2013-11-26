#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "../../../libs/stft/fft.h"

using namespace std;

int main()
{
    class Row {
    public:
        size_t timeStamp;
        vector<float> cols;
    };
    Row row;
    row.cols.resize(64);

    //ofstream spectroFile("/media/ssd/outSpectro.bin", ios::out | ios::binary);
    FILE* pFile;
    pFile = fopen("/media/ssd/outSpectro.bin", "wb");
    setvbuf(pFile, NULL, _IOFBF, sizeof(float)*(24400/2+1));

    FILE* pFileForce;
    pFileForce = fopen("/media/ssd/outForce.bin", "wb");

    FILE* pFilePower;
    pFilePower = fopen("/media/ssd/outPower.bin", "wb");

    unsigned int fftWinSizeSamples = 24400/2;
    unsigned int samplingRate = 24400;
    unsigned int numChannels = 30;
    unsigned freqRange = fftWinSizeSamples / 2 + 1;
    Fft<float> fft(fftWinSizeSamples, Fft<float>::windowFunc::BLACKMAN_HARRIS, samplingRate, numChannels);

    ifstream dataFile("/media/ssd/penn_data/data_Thu_21.11.2013_12:32:02", ios::in | ios::binary);

    vector<float> points(30);

    vector<vector<float> > powers(30);
    for (unsigned i=0; i<30; i++){
        powers[i].resize(freqRange);
    }

    unsigned int bin70 = int(75.0 / (samplingRate/fftWinSizeSamples));
    unsigned int bin100 = int(90.0 / (samplingRate/fftWinSizeSamples));

    float alpha = 1.0;
    float movingAvg = 0.0;

    while(dataFile) {
        dataFile.read((char *) &(row.timeStamp), sizeof(size_t));
        dataFile.read((char *) (row.cols.data()), sizeof(float) * 64);
        if ((row.timeStamp % 24400) == 0)
            cout<<row.timeStamp<<endl;

        memcpy(points.data(), row.cols.data()+4, sizeof(float) * 30);

        fft.AddPoints(points);

        if (row.timeStamp % (24400/20) == 0)
        if (fft.Process()) {
            //cout << "t: " << row.timeStamp << endl;
            fft.GetPower(powers);

            for (unsigned i=0; i<30;i++) {
                //spectroFile.write((const char*)(powers[i].data()), sizeof(float)*(samplingRate/2+1));
                fwrite(powers[i].data(), sizeof(float), freqRange, pFile);
            }

            float avgPower = 0.0;
            for (unsigned i=3; i<13; i++) {
                for (unsigned j=bin70;j<bin100;j++) {
                    avgPower += powers[i][j];
                }
            }

            movingAvg = alpha * avgPower + (1.0 - alpha) * movingAvg;

            fwrite(row.cols.data(), sizeof(float), 1, pFileForce);
            fwrite(&movingAvg, sizeof(float), 1, pFilePower);
        }

    }

    fclose(pFile);
    fclose(pFilePower);
    fclose(pFileForce);
    return 0;
}

