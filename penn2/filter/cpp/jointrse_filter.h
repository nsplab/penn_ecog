#ifndef JOINTRSE_FILTER_H
#define JOINTRSE_FILTER_H

#include "reachstateequation.h"
#include "filter_class.h"

class jointRSE_filter : public FilterClass {
public:
    jointRSE_filter(size_t dim, bool velocityParams=true, bool positionParams=true, bool useRSE=true, bool log=false);
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

    void LogInnovation(arma::vec obs, arma::mat estimatedObs);

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

    size_t dim_;
    bool velocityParams_;
    bool positionParams_;

    arma::cube DD_obs_;

    static const size_t numLags = 2;
    static const size_t numChannels = 6;
    std::ofstream channelParamsFile;
    std::ofstream innovationFile;

    bool log_;
};

#endif // JOINTRSE_FILTER_H
