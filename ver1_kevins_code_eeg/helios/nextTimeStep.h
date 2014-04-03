#ifndef NEXTTIMESTEP_H
#define NEXTTIMESTEP_H

#include <armadillo>
using namespace arma;

void InitFilter();
void nextTimeStep(vec &position, int &eegInstruction, \
                  std::map<std::string, cube> &graphingMap, \
                  vec const &eegSignals);

#endif
