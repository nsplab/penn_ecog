#include "jointrse_filter.h"
#include "matrix.h"

#include <unistd.h>

using namespace arma;

// todo: replace cube with vector of matrices

jointRSE_filter::jointRSE_filter(size_t dim, bool velocityParams, bool positionParams, bool affineParam, bool useRSE, bool timeInvariant, bool log):
    timeStep_(-1), prevTrialId_(0), velocityParams_(velocityParams), positionParams_(positionParams), affineParam_(affineParam), timeInvariant_(timeInvariant), log_(log)
{
    dim_ = dim;

    // make sure at least one of set of parameters is used in filter
    assert(velocityParams_ | positionParams_);

    numSetsOfParams_ = (size_t) velocityParams_ + (size_t) positionParams_; // Intentionally does not contain affineParam
    cout<<"numSetsOfParams_ "<<numSetsOfParams_<<endl;

    channelParamsFile.open("channelParams.txt");
    innovationFile.open("innovation.txt");
    covarianceFile.open("covariance.txt");

    pos_ = zeros<mat>(dim, 1);
    prev_u_ = zeros<mat>(dim, 1);
    for (size_t i = 0; i <= sensoryDelay; i++) {
        saved_pos_.push_back(pos_);
        saved_u_.push_back(prev_u_);
    }

    const double timeBin = 0.01;
    // page 31
    const double maxTimeSteps = 3.0/timeBin;
    // page 27
    int reachTimeSteps = 1.0/timeBin;
    // dim*2: position+velocity
    mat reachTarget = zeros<mat>(dim*2, 1);


    RSEMatrixStruct rseParams;
    if (timeInvariant_) {
        const double alpha = 0.18;
        const double beta = 0.1;
        const double gamma = 0.1;
        mat Q = zeros<mat>(2 * dim, 2 * dim);
        for (size_t i = 0; i < dim; i++) {
            Q(i, i) = alpha;
            Q(dim + i, dim + i) = beta;
        }
        mat R = gamma * eye<mat>(dim, dim);

        timeInvariantRSE rseComputer(reachTarget, Q, R, dim);
        rseParams = rseComputer.returnAnswer();
    } else {
        reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget, dim);
        rseParams = rseComputer.returnAnswer();
    }

    // uHistory: history of kinematic params, first position and then velocity
    uHistory_ = zeros<mat>(dim * numSetsOfParams_ * numLags + affineParam_, 1);

    mat FSingleTimeChannels = eye<mat>(dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels,
                                       dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels);
    cube FChannels = repslices(FSingleTimeChannels, timeInvariant_ ? 1 : maxTimeSteps);

    // Eq. 18
    F_ = blkdiag(FChannels, rseParams.F);
    cout<<"F_ size: "<<F_.n_rows<<" "<<F_.n_cols<<endl;

    mat QSingleTimeChannels = zeros<mat>(dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels,
                                         dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels);
    cube QChannels = repslices(QSingleTimeChannels, timeInvariant_ ? 1 : maxTimeSteps);

    // Eq.19
    Q_ = blkdiag(QChannels, rseParams.Q);

    const double channelCov = 1.0e-2;
    mat initialChannelCov = channelCov * eye<mat>(dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels,
                                                  dim * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels);

    covariance_ = prepareINITIAL_ARM_COV(timeBin);
    covariance_ = blkdiag(initialChannelCov, covariance_);

    //b_ = zeros<cube>(numChannels * FChannels.n_rows + rseParams.b.n_rows, 1, maxTimeSteps);
    //b_.subcube(numChannels * FChannels.n_rows, 0, 0, b_.n_rows-1, b_.n_cols-1, b_.n_slices - 1) = rseParams.b;

    b_ = zeros<cube>(FChannels.n_rows + rseParams.b.n_rows, 1, timeInvariant_ ? 1 : maxTimeSteps);

    // TODO                      v---should this be FChannels.n_cols?
    b_.subcube(FChannels.n_rows, 0, 0, b_.n_rows-1, b_.n_cols-1, b_.n_slices - 1) = rseParams.b;

    // TODO: start with something of the right order of magnitude
    channelParametersHat_ = zeros<mat>(numChannels, dim * numSetsOfParams_ * numLags + affineParam_);
}

void jointRSE_filter::Predict() {
    timeStep_ ++;

    // INITIALIZE THE STATE VECTOR. The first elements are the channel
    // parameters, presented in order of channel number, then x-y-z
    // directionality (x then y then z), and finally lag number (least
    // lag to greatest lag). Then comes position (x-y-z) and decoded
    // velocity history, presented in order of x-y-z directionality,
    // then lag number.
    // NOTE: The third argument 1 to reshape signifies row-wise reshaping. I
    //       think.
    //cout<<"preparing to initialize state"<<endl;

    // mat prev_u = zeros(dim_,1);
    // //cout<<"uHistory n_rows " << uHistory.n_rows << endl;
    // if (velocityParams_) {
    //     size_t offset = 0;
    //      if (positionParams_) {
    //         offset = dim_ * numLags;
    //     }
    //     for (size_t i=0; i<dim_; i++) {
    //         prev_u(i,0) = uHistory_(offset + i*numLags, 0);
    //     }
    // }

    //cout<<"uHistory n_rows " << uHistory.n_rows << endl;
    //prev_u << uHistory_(0, 0) << endr << uHistory_(numSetsOfParams_ * numLags, 0) << endr
    //       << uHistory_(2 * numSetsOfParams_ * numLags, 0) << endr;


    //cout<<"prev_u successfully initialized"<<endl;
    //cout<<"channelParametersHat n_rows " << channelParametersHat_.n_rows << "n_cols " << channelParametersHat_.n_cols<< endl;
    mat x = join_cols(join_cols(
        reshape(channelParametersHat_, dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels, 1, 1), pos_),
        prev_u_);
    //cout<<"initialized state"<<endl;

    // Select the correct matrices.
    mat F_current = F_.slice(timeInvariant_ ? 0 : timeStep_);
    mat Q_current = Q_.slice(timeInvariant_ ? 0 : timeStep_);
    mat b_current = b_.slice(timeInvariant_ ? 0 : timeStep_);

    // PERFORM THE PREDICTION STEP.
    pred_x_ = F_current * x + b_current;
    pred_cov_ = F_current * covariance_ * F_current.t() + Q_current;

    /*cout<<"Q_current size: "<<Q_current.n_rows<<" "<<Q_current.n_cols<<endl;
    cout<<"b_current size: "<<b_current.n_rows<<" "<<b_current.n_cols<<endl;
    cout<<"x_ size: "<<x.n_rows<<" "<<x.n_cols<<endl;
    cout<<"F size: "<<F_current.n_rows<<" "<<F_current.n_cols<<endl;
    cout<<"covariance_ size: "<<covariance_.n_rows<<" "<<covariance_.n_cols<<endl;
    cout<<"pred_x_ size: "<<pred_x_.n_rows<<" "<<pred_x_.n_cols<<endl;
    cout<<"pred_cov_ size: "<<pred_cov_.n_rows<<" "<<pred_cov_.n_cols<<endl;*/
}

void jointRSE_filter::Update() {
    //usleep(100000);

    // add the construction of dd matrices here hard code for each case of 2d/3d/position/velocity
    // a function structure of matrices are fixed

    // PERFORM THE UPDATE STEP.
    // predicted_uHistory, vector of current+history of kinematic members of state vector
    mat predicted_uHistory = zeros<mat>(uHistory_.n_rows, uHistory_.n_cols);
    // ???
    // // first, set last predicted kinematic values
    // size_t vOffset = 0;
    //  if (! positionParams_) {
    //     vOffset = dim_;
    // }
    // for (size_t i=0; i<dim_*numSetsOfParams_; i++) {
    //     predicted_uHistory(i*numLags, 0) = pred_x_(dim_*numSetsOfParams_ * numLags * numChannels + vOffset + i, 0);
    // }
    // // second, set history of predicted kinematic values
    // for (size_t i=0; i<dim_*numSetsOfParams_; i++) {
    //     predicted_uHistory.submat(i*numLags+1, 0, (i+1)*numLags - 1, 0) = uHistory_.submat(i*numLags, 0, (i+1)*numLags - 2, 0);
    // ???
    for (int i = 0; i < dim_; i++) {
        if (positionParams_ ) {
            predicted_uHistory(i * numLags, 0) = pred_x_(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + i, 0);
            if (numLags != 1) {
                predicted_uHistory.submat(i * numLags + 1, 0, (i + 1) * numLags - 1, 0) =
                         uHistory_.submat(i * numLags,     0, (i + 1) * numLags - 2, 0);
            }
        }
        if (velocityParams_) { // Not "else if" (can have both position and velocity)
            predicted_uHistory(positionParams_ * dim_ * numLags + i * numLags, 0) = pred_x_(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + dim_ + i, 0);
            if (numLags != 1) {
                predicted_uHistory.submat(positionParams_ * dim_ * numLags + i * numLags + 1, 0, positionParams_ * dim_ * numLags + (i + 1) * numLags - 1, 0) =
                         uHistory_.submat(positionParams_ * dim_ * numLags + i * numLags,     0, positionParams_ * dim_ * numLags + (i + 1) * numLags - 2, 0);
            }
        }
    }
    if (affineParam_) {
        predicted_uHistory(numSetsOfParams_ * dim_ * numLags, 0) = 1;
    }

    // eq. 1.1 in write up
    // u (mu) eq. 3.11 in write up is estimated_obs
    mat estimated_obs = channelParametersHat_ * predicted_uHistory;

    // The derivative of the observation vector with respect to the state,
    // evaluated at estimated_obs.
    // Eq. 3.10 (write up)
    mat D_obs = zeros<mat>(pred_x_.n_rows, numChannels);
    for(size_t c=0; c<numChannels; c++)
    {
        if (positionParams_ ) {
            D_obs.submat(c * dim_ * numLags, c, (c + 1) * dim_ * numLags - 1, c) =
                    predicted_uHistory.submat(0, 0, dim_ * numLags - 1, 0);
            for (int i = 0; i < dim_; i++) {
                D_obs(D_obs.n_rows - dim_ - dim_ + i, c) = pred_x_(c * dim_ * numLags + i * numLags, 0);
            }
        }
        if (velocityParams_) { // Not "else if" (can have both position and velocity)
            D_obs.submat(positionParams_ * numChannels * dim_ * numLags + c * dim_ * numLags, c, positionParams_ * numChannels * dim_ * numLags + (c + 1) * dim_ * numLags - 1, c) =
                    predicted_uHistory.submat(positionParams_ * dim_ * numLags, 0, positionParams_ * dim_ * numLags + dim_ * numLags - 1, 0);
            for (int i = 0; i < dim_; i++) {
                D_obs(D_obs.n_rows - dim_ + i, c) = pred_x_(positionParams_ * numChannels * dim_ * numLags + c * dim_ * numLags + i * numLags, 0);
            }
        }
        if (affineParam_) {
            D_obs(numSetsOfParams_ * numChannels * dim_ * numLags, c) = 1;
        }
    }

    // The double derivative of the observation vector with respect to the
    // state, evaluated at estimated_obs.
    //cout<<"Computing DD_OBS"<<endl;
    // Eq. 3.13 (write up)
    cube DD_obs = zeros<cube>(pred_x_.n_rows, pred_x_.n_rows, numChannels);
    for(size_t c=0; c<numChannels; c++)
    {
        for (int i = 0; i < dim_; i++) {
            if (positionParams_ ) {
                DD_obs(c * dim_ * numLags + i * numLags, DD_obs.n_cols - dim_ - dim_ + i, c) = 1;
                DD_obs(DD_obs.n_cols - dim_ - dim_ + i, c * dim_ * numLags + i * numLags, c) = 1;
            }
            if (velocityParams_) { // Not "else if" (can have both position and velocity)
                DD_obs(positionParams_ * numChannels * dim_ * numLags + c * dim_ * numLags + i * numLags, DD_obs.n_cols - dim_ + i, c) = 1;
                DD_obs(DD_obs.n_cols - dim_ + i, positionParams_ * numChannels * dim_ * numLags + c * dim_ * numLags + i * numLags, c) = 1;
            }
            if (affineParam_) {
                // Nothing to do
                // Taking two derivatives will always result in a 0 (regardless of which two are selected)
            }
        }
    }
    //cout<<"Done computing DD_OBS"<<endl;

    // What we add to the inverse of the predicted value of the covariance to
    // obtain the updated value.
    //cout<<"Computing cov_adjust"<<endl;
    // part of Eq. 3.12 (write up)
    const double baseVariance = 1.0;
    mat channelVariances = baseVariance * ones<mat>(numChannels, 1);
    mat cov_adjust = zeros(pred_cov_.n_rows, pred_cov_.n_cols);
    for(size_t c = 0; c < numChannels; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
            cov_adjust += 1 / channelVariances(c) *
                (D_obs.col(c) * trans(D_obs.col(c)) +
                (estimated_obs(c, 0) - obs_(c)) * DD_obs.slice(c));
    }
    //cout<<"Done computing cov_adjust"<<endl;
    //vec test_singular_values = svd(pred_cov);
    //cout<<"singular values of predicted covariance: "<<test_singular_values<<endl;
    // Eq. 3.12 (write up)
    //cout<<"pred_cov_: "<<pred_cov_<<endl;
    //cout<<"covariance_: "<<covariance_<<endl;
    //cout<<"cov size "<<covariance_.n_rows<<" "<<covariance_.n_cols<<endl;

    mat new_cov_inv = inv(pred_cov_) + cov_adjust;
    cout<<"Done computing new_cov_inv"<<endl;

    // Check to see if new_cov_inv is well-conditioned.
    vec singular_values = svd(new_cov_inv);
    mat new_cov;
    double rcond =
        singular_values(0) / singular_values(singular_values.n_elem - 1);
    if(rcond >= 1.0e-8)
    {
        new_cov = inv(new_cov_inv);
    }
    else
    {
        std::cout<<"Matrix is ill-conditioned: "<<rcond<<"; using predicted"
            <<" covariance instead"<<std::endl;
        new_cov = pred_cov_;
    }
    //cout<<"done computing new_cov"<<endl;

    // What we add to the predicted value of the state to obtain the updated
    // value.
    //cout<<"Computing x_adjust"<<endl;
    mat x_adjust = zeros(pred_x_.n_rows, pred_x_.n_cols);
    mat innovation = zeros(numChannels, 1);
    // part of Eq. 3.11 (write up)
    for(size_t c = 0; c < numChannels; c++)
    {
        if(!isnan(estimated_obs(c, 0)))
        {
            x_adjust += 1 / channelVariances(c) *
                (obs_(c) - estimated_obs(c, 0)) * D_obs.col(c);
            innovation(c, 0) =
                    (obs_(c) -  estimated_obs(c, 0)) / channelVariances(c);
        }
        cout<<"novel_"<<c<<": "<<(obs_(c) -  estimated_obs(c, 0))<<endl;
    }

    if (log_) {
        LogInnovation(obs_,estimated_obs);
    }
    //cout<<"Done computing x_adjust"<<endl;
    // Eq. 3.11 (write up)
    mat new_x = pred_x_ + new_cov * x_adjust;
    //cout<<"Done computing new_x"<<endl;

    // UPDATE THE CLASS VARIABLES
    mat new_channelParametersHat = new_x.submat(0, 0,
                                                dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels- 1, 0);
    channelParametersHat_ =
        trans(reshape(new_channelParametersHat, dim_ * numSetsOfParams_ * numLags + affineParam_, numChannels, 1));
    channelParamsFile<<channelParametersHat_<<endl;
    // for(size_t i=0; i<(channelParametersHat_.n_rows); i++)
    //     for(size_t j=0; j<(channelParametersHat_.n_cols); j++)
    //         channelParamsFile<<channelParametersHat_(i,j)<<" ";
    // channelParamsFile<<";"<<endl;

    // extract position from updated state vector
    pos_ = new_x.submat(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels, 0,
                        dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + dim_ - 1, 0);
    prev_u_ = new_x.submat(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + dim_, 0,
                           dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + dim_ + dim_ - 1, 0);
    saved_pos_.push_back(pos_);
    saved_u_.push_back(prev_u_);
    pos_ = saved_pos_[0];
    prev_u_ = saved_u_[0];
    saved_pos_.erase(saved_pos_.begin());
    saved_u_.erase(saved_u_.begin());

    // to test
    vec handState = new_x.submat(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels, 0,
                                 dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + 2 * dim_ - 1, 0);

    handState_ = conv_to< std::vector<float> >::from(handState);

    cout<<"pos_: "<<pos_<<endl;

    for (int i = 0; i < dim_; i++) {
        if (positionParams_ ) {
            uHistory_(i * numLags, 0) = new_x(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + i, 0);
        }
        if (velocityParams_) { // Not "else if" (can have both position and velocity)
            uHistory_(positionParams_ * dim_ * numLags + i * numLags, 0) = new_x(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels + dim_ + i, 0);
        }
    }
    if (affineParam_) { // Not "else if" (can have both position and velocity)
        // This term is probably always going to be 1 (I don't think anything should change it)
        // Just to be safe...
        uHistory_(numSetsOfParams_ * dim_ * numLags, 0) = 1;
    }

    const double timeBin = 0.01;
    //if(covReset.compare("yes") == 0)
        covariance_ = blkdiag(new_cov.submat(0, 0, dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels - 1,
                                                   dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels - 1), prepareINITIAL_ARM_COV(timeBin));
        covarianceFile<<covariance_<<endl;
        //covariance_ = new_cov;
    //else if(covReset.compare("posOnly") == 0)
    //    covariance = blkdiag(blkdiag(
    //        new_cov.submat(0, 0, 3 * numLags * numChannels - 1,
    //        3 * numLags * numChannels - 1), prepareINITIAL_ARM_COV(timBin).submat(0, 0, 2, 2)),
    //        new_cov.submat(new_cov.n_rows - 3, new_cov.n_cols - 3,
    //        new_cov.n_rows - 1, new_cov.n_cols - 1));
    //else if(covReset.compare("no"))
    //    covariance_ = new_cov;
}

void jointRSE_filter::InitNewTrial(mat startPos, mat reachTarget) {
    pos_ = startPos;
    prev_u_ = zeros<mat>(dim_, 1);
    saved_pos_.clear();
    saved_u_.clear();
    for (size_t i = 0; i <= sensoryDelay; i++) {
        saved_pos_.push_back(pos_);
        saved_u_.push_back(prev_u_);
    }
    uHistory_ = zeros<mat>(dim_ * numSetsOfParams_ * numLags + affineParam_, 1);

    const double timeBin = 0.01;
    covariance_ = blkdiag(covariance_.submat(0, 0, dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels - 1,
                                                   dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels - 1), prepareINITIAL_ARM_COV(timeBin));

    timeStep_ = -1;

    // page 31
    const double maxTimeSteps = 3.0/timeBin;
    // page 27
    int reachTimeSteps = 1.0/timeBin;
    reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget, dim_);
    RSEMatrixStruct rseParams;
    rseParams = rseComputer.returnAnswer();
    mat FSingleTimeChannels = eye<mat>(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels,
                                       dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels);
    cube FChannels = repslices(FSingleTimeChannels, timeInvariant_ ? 1 : maxTimeSteps);
    // Eq. 18
    F_ = blkdiag(FChannels, rseParams.F);
    cout<<"F_ size: "<<F_.n_rows<<" "<<F_.n_cols<<endl;

    mat QSingleTimeChannels = zeros<mat>(dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels,
                                         dim_ * numSetsOfParams_ * numLags * numChannels + affineParam_ * numChannels);
    cube QChannels = repslices(QSingleTimeChannels, timeInvariant_ ? 1 : maxTimeSteps);

    // Eq.19
    Q_ = blkdiag(QChannels, rseParams.Q);

    //b_ = zeros<cube>(numChannels * FChannels.n_rows + rseParams.b.n_rows, 1, maxTimeSteps);
    //b_.subcube(numChannels * FChannels.n_rows, 0, 0, b_.n_rows-1, b_.n_cols-1, b_.n_slices - 1) = rseParams.b;

    b_ = zeros<cube>(FChannels.n_rows + rseParams.b.n_rows, 1, timeInvariant_ ? 1 : maxTimeSteps);

    // TODO                      v---should this be FChannels.n_cols?
    b_.subcube(FChannels.n_rows, 0, 0, b_.n_rows-1, b_.n_cols-1, b_.n_slices - 1) = rseParams.b;
}

void jointRSE_filter::Run() {
    for (;;) {
        GrabFeatures();

        SendHandPosGetState(handPos_);

        obs_.resize(features_.size());
        for (size_t i=0; i<features_.size(); i++) {
            obs_[i] = features_[i];
        }

        // re-initialize filter at start of new trial
        if (prevTrialId_ != trial_id) {
            cout<<"new trial started"<<endl;
            pos_.resize(handPos_.size());
            for (size_t i=0; i<handPos_.size(); i++) {
                pos_[i] = handPos_[i];
            }

            arma::mat reachTarget = zeros<mat>(dim_*2, 1);
            for (size_t i=0; i<target_.size(); i++) {
                reachTarget[i] = target_[i];
            }

            InitNewTrial(pos_, reachTarget);
            prevTrialId_ = trial_id;
        }

        Predict();
        Update();

        handPos_[0] += pos_[0];
        handPos_[1] += pos_[1];
        handPos_[2] += pos_[2];
    }
}

// The initial covariance on the arm components of the state.
// page 20
mat jointRSE_filter::prepareINITIAL_ARM_COV(const double timeBin) {
    mat ans = zeros<mat>(2 * dim_, 2 * dim_);
    double posCov = 1.0e-7;
    double velCov = 1.0e-7 / timeBin;
    for (size_t i = 0; i < dim_; i++) {
        ans(i, i) = posCov;
        ans(dim_ + i, dim_ + i) = velCov;
    }

    return ans;
}

void jointRSE_filter::LogInnovation(arma::vec obs, arma::mat estimatedObs) {
    for(size_t c = 0; c < numChannels; c++)
    {
        innovationFile<<(obs_(c) - estimatedObs(c, 0))<<" ";
    }
    innovationFile<<";"<<endl;
}

