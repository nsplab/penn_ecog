#ifndef JOINTRSE_FILTER_H
#define JOINTRSE_FILTER_H

#include "reachstateequation.h"
#include "filter_class.h"

class jointRSE_filter : public FilterClass {
public:
    static const size_t numLags = 1;
    static const size_t numChannels = 1;
    static const size_t sensoryDelay = 0;

    jointRSE_filter(size_t dim, bool velocityParams=true, bool positionParams=true, bool affineParam=true, bool useRSE=true, bool timeInvariant=false, bool log=false);
    void Update();
    void Predict();
    // main loop function
    void Run();
private:
    arma::mat prepareINITIAL_ARM_COV(const double timeBin);
    void InitNewTrial(arma::mat startPos, arma::mat reachTarget);

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
    std::vector<arma::mat> saved_pos_;
    arma::mat prev_u_;
    std::vector<arma::mat> saved_u_;
    arma::mat pred_x_;
    arma::mat pred_cov_;
    arma::vec obs_;

    bool velocityParams_;
    bool positionParams_;
    bool affineParam_;
    size_t numSetsOfParams_;
    bool timeInvariant_;

    arma::cube DD_obs_;

    std::ofstream channelParamsFile;
    std::ofstream innovationFile;
    std::ofstream covarianceFile;

    bool log_;
};

#endif // JOINTRSE_FILTER_H
