#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3) {
        cout<<"run fixbrokenfile <broken file> <corrected file>"<<endl;
        return 0;
    }

    ifstream ifile(argv[1], ios::in|ios::binary|ios::ate);

    if (! ifile.is_open()) {
        cout<<"couldn't opent the input file"<<endl;
        return 0;
    }

    streampos size;
    size = ifile.tellg();

    float* memblock = new float[size/sizeof(float)];

    size_t totalSamples = size/sizeof(float);

    cout<<"totalSamples: "<<totalSamples<<endl;

    ifile.seekg(0, ios::beg);
    ifile.read((char*)memblock, size);
    ifile.close();

    vector<size_t> numberOfSamples;

    bool prevWasZero = false;

    size_t totalExpectedSamples = 0;

    size_t max = 0;

    size_t currentNumZeros = 0;
    for (size_t i=0; i<totalSamples; i++) {
        if (prevWasZero) {
            if (memblock[i] == 0.0f) {
                currentNumZeros += 1;
            } else {
                prevWasZero = false;
                if ((currentNumZeros % 3) == 0) {
                    numberOfSamples.push_back(currentNumZeros/3);
                    totalExpectedSamples += currentNumZeros/3;
                    if (currentNumZeros > max)
                        max = currentNumZeros;
                    i += currentNumZeros/3 * 60 + 0;
                    currentNumZeros = 0;
                    continue;
                }
                currentNumZeros = 0;
            }
        } else {
            if (memblock[i] == 0.0f) {
                prevWasZero = true;
                currentNumZeros += 1;
            }
        }
    }

    cout<<"max: "<<max<<endl;

    totalExpectedSamples -= numberOfSamples[numberOfSamples.size()-1];

    cout<<"totalExpectedSamples: "<<totalExpectedSamples<<endl;

    vector<vector<float> > channels(64);

    cout<<"s: "<<numberOfSamples.size()<<endl;

    size_t currentIdx = 0;
    for (size_t i=0; i<(numberOfSamples.size()-340000); i++) {
        //cout<<"n: "<<numberOfSamples[i]<<endl;
        for (size_t ch=0; ch<64; ch++) {
            //cout<<"ch: "<<ch<<endl;
            for (size_t sample=0; sample<numberOfSamples[i]; sample++) {
                //cout<<"sample: "<<sample<<endl;

                if (currentIdx >= totalSamples)
                    cout<<currentIdx<<endl;

                channels[ch].push_back(memblock[currentIdx]);
                currentIdx += 1;
            }
        }
    }

    cout<<"currentIdx: "<<currentIdx<<endl;

    ofstream ofile(argv[2], ios::out|ios::binary);

    for (size_t sample=0; sample<channels[0].size(); sample++) {
        for (size_t i=0; i<64; i++) {
            ofile.write((char*)(&(channels[i][sample])), sizeof(float));
        }
    }
    ofile.close();
    cout<<"the fixed file is ready"<<endl;

    return 0;
}

