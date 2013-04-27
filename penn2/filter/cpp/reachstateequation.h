#ifndef REACHSTATEEQUATION_H
#define REACHSTATEEQUATION_H

#include <armadillo>


class reachStateEquation
{
public:
    reachStateEquation(size_t maxTimeSteps, size_t reachTimeSteps, arma::mat reachTarget);
    // system evolution matrix that decribes how the kinematic components are
    // expected to evolve over time.
    arma::mat prepareF_ARM_UNDIRECTED();
    // increment covariance matrix that describes how the kinematics of the arm
    // are expected to be affected by Gaussian noise in each time step.
    arma::mat prepareQ_ARM_UNDIRECTED();
    arma::mat prepareREACH_TARGET_COVARIANCE();
    arma::cube repslices(arma::mat matrix, int n_slices);
    struct RSEMatrixStruct {
        arma::cube F, Q, b;
    };
    RSEMatrixStruct answer_;
    RSEMatrixStruct returnAnswer(){return answer_;}
private:
    static const double timeBin = 0.01;
};

#endif // REACHSTATEEQUATION_H