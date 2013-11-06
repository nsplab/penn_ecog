#ifndef JOINTRSE_FILTER_H
#define JOINTRSE_FILTER_H

#include "reachstateequation.h"
#include "filter_class.h"

class jointRSE_filter : public FilterClass {
public:
    static const size_t numChannels = 1;
    static const size_t sensoryDelay = 5;

    jointRSE_filter(size_t dim, bool velocityParams, bool positionParams, bool affineParam, bool useRSE,
                    bool timeInvariant, bool log, float trialTime, float maxTrialTime, double diagQ, double finalPosCov,
                    double finalVelCov, unsigned featureRate, double channelCov, double initialArmPosVar, double initialArmVelVar,
                    bool integrateVel, unsigned numLags);
    void Update();
    void Predict();
    // main loop function
    void Run();
    void RunPredictOnly();
private:
    arma::mat prepareINITIAL_ARM_COV();
    void InitNewTrial(arma::mat startPos, arma::mat reachTarget);

    void LogInnovation(arma::mat estimatedObs);
    void LogStateVector(arma::mat state);
    void LogUpdateStateVector(arma::mat state);

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

    double maxTimeSteps_;
    int reachTimeSteps_;
    double timeBin_;

    bool integrateVel_;

    double initialArmPosVar_;
    double initialArmVelVar_;

    bool velocityParams_;
    bool positionParams_;
    bool affineParam_;
    size_t numSetsOfParams_;
    bool timeInvariant_;

    unsigned numLags_;

    arma::cube DD_obs_;

    std::ofstream channelParamsFile;
    std::ofstream innovationFile;
    std::ofstream covarianceFile;
    std::ofstream stateFile;
    std::ofstream updateStateFile;

    bool log_;
};

#endif // JOINTRSE_FILTER_H
