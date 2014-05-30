#ifndef REACHSTATEEQUATION_H
#define REACHSTATEEQUATION_H

#include <armadillo>

struct RSEMatrixStruct {
    arma::cube F, Q, b;
};

class reachStateEquation
{
public:
    // TODO: Don't actually need dim (could just use length of reachTarget)
    reachStateEquation(size_t maxTimeSteps, size_t reachTimeSteps, arma::mat reachTarget, size_t dim = 3, double diagQ=1.0e-3, double finalPosCov=1.0e-6, double finalVelCov=1.0e-8, double timeBin=0.01);
    // system evolution matrix that decribes how the kinematic components are
    // expected to evolve over time.
    arma::mat prepareF_ARM_UNDIRECTED(size_t dim);
    // increment covariance matrix that describes how the kinematics of the arm
    // are expected to be affected by Gaussian noise in each time step.
    arma::mat prepareQ_ARM_UNDIRECTED(size_t dim, double velInc);
    arma::mat prepareREACH_TARGET_COVARIANCE(size_t dim, double finalPosCov, double finalVelCov);

    RSEMatrixStruct answer_;
    RSEMatrixStruct returnAnswer(){return answer_;}
private:
    double timeBin_;
};

class timeInvariantRSE
{
public:
    // TODO: Don't actually need dim (could just use length of reachTarget)
    timeInvariantRSE(arma::mat reachTarget, arma::mat Q, arma::mat R, size_t dim = 3);
    // system evolution matrix that decribes how the kinematic components are
    // expected to evolve over time.
    arma::mat prepareF_ARM_UNDIRECTED(size_t dim);
    // increment covariance matrix that describes how the kinematics of the arm
    // are expected to be affected by Gaussian noise in each time step.
    arma::mat prepareQ_ARM_UNDIRECTED(size_t dim);
    arma::mat prepareREACH_TARGET_COVARIANCE(size_t dim);

    RSEMatrixStruct answer_;
    RSEMatrixStruct returnAnswer(){return answer_;}
private:
    double timeBin;
};

#endif // REACHSTATEEQUATION_H
