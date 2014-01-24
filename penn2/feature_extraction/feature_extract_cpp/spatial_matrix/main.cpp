// ./spatial_matrix 3 64 1 2 3 eye_3_64.csv

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv)
{
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);

    cout<<"rows: "<<rows<<endl;
    cout<<"cols: "<<cols<<endl;

    int c1 = atoi(argv[3]);
    int c2 = atoi(argv[4]);
    int c3 = atoi(argv[5]);

    ofstream ofile(argv[6]);

    for (unsigned i=0; i<rows; i++) {
        for (unsigned j=0; j<cols; j++) {
            if (i == 0) {
                if (j == c1)
                    ofile<<"1.0 ";
                else
                    ofile<<"0.0 ";
            } else if (i == 1) {
                if (j == c2)
                    ofile<<"1.0 ";
                else
                    ofile<<"0.0 ";
            } else if (i == 2) {
                if (j == c3)
                    ofile<<"1.0 ";
                else
                    ofile<<"0.0 ";
            }
        }
        ofile<<endl;
    }

    return 0;
}

