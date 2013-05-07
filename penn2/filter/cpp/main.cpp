#include <iostream>
#include <vector>
#include "naive_filter.h"
#include "reachstateequation.h"
#include "jointrse_filter.h"
#include "tests.h"

using namespace std;
using namespace arma;

void testRSE();
void testJointFilter();

int main()
{
    //cout << "Filter is running" << endl;

    //NaiveFilter naive_filter;
    //naive_filter.Run();

    //testRSE();
    testJointFilter();

    return 0;
}
