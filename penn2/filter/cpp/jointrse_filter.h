#ifndef JOINTRSE_FILTER_H
#define JOINTRSE_FILTER_H

#include "reachstateequation.h"
#include "filter_class.h"

class jointRSE_filter : public FilterClass {
public:
    jointRSE_filter();
    void Update();
    void Predict();
    // man loop function
    void Run();
private:
    arma::cube repslices(arma::mat matrix, int n_slices);
    arma::cube blkdiag(arma::cube A, arma::cube B);
    arma::mat blkdiag(arma::mat A, arma::mat B);
    arma::mat prepareINITIAL_ARM_COV(const double timeBin);
    void InitNewTrial(arma::mat startPos);

    // state evolution matrix
    arma::cube F_;
    arma::cube Q_;
    arma::mat covariance_;
    arma::cube b_;
    int timeStep_;
    arma::mat uHistory_;
    arma::mat channelParametersHat_;
    size_t prevTrialId_;
    arma::mat pos_;
    arma::mat pred_x_;
    arma::mat pred_cov_;
    arma::vec obs_;

    static const int numLags = 5;
    static const int numChannels = 14;
};

#endif // JOINTRSE_FILTER_H
