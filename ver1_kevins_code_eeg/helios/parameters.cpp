
#include <armadillo>
#include <string>

using namespace arma;

/* SYSTEM PARAMETERS */

// Number of EEG channels that are being used to obtain signals.
int N_CHANNELS = 14;

// Number of velocity lag terms that are present in the signal model, including
// the one from the present time step.
int N_LAGS = 5;

// Length of each time bin, in seconds. 
double TIME_BIN = 0.01;

// The number of baseline samples that are taken to compute the mean and variance
// of the signal at each channel.
int N_BASELINE_SAMPLES = 1000;

/* TRAINING RUN PARAMETERS */

// The number of trials in each training run.
int N_TRIALS = 26;

// An array of booleans that determines which trials are test trials and which
// are training trials. If TEST_TRIALS[i] is true, then trial 'i' is a test
// trial, and if TEST_TRIALS[i] is false then trial 'i' is a training trial.
int nTestTrials = 6; //0;
bool* prepareTEST_TRIALS()
{
    int everyNthTrial = 5;

    bool ans[N_TRIALS];
    for(int i=0; i<N_TRIALS; i++)
    {
       ans[i] = (i % everyNthTrial == 0);
       if(ans[i])
           nTestTrials++;
    }

    return &(ans[0]);
}

bool ans[] = {true, false, false, false, false, true, \
                    false, false, false, false, true, \
                    false, false, false, false, true, \
                    false, false, false, false, true, \
                    false, false, false, false, true};
bool* TEST_TRIALS = &(ans[0]); //prepareTEST_TRIALS();

// The number of test and training trials in each training run. Derived from
// TEST_TRIALS.
int N_TEST_TRIALS = nTestTrials;
int N_TRAIN_TRIALS = N_TRIALS - N_TEST_TRIALS;

// This is the number of seconds that the simulation should pause before the
// start of a new trial so that the user can see where his rendered hand is in
// relation to the target.
int TIME_BETWEEN_TRIALS = 2;

/* TRIAL PARAMETERS */
// This is the distance, in meters, that the hand starts from the center of the
// target.
double START_MAG = 0.2;

// This is the radius of the spherical target, in meters.
double TARGET_RADIUS = 0.05;

// This is the maximum number of time steps that the trial will go on for.
int MAX_TIME_STEPS = (int) (3 / TIME_BIN);

// This is the number of time steps that the hand must remain with TARGET_RADIUS
// of the target for the trial to end in success.
int HOLD_TIME_STEPS = (int) (0.5 / TIME_BIN);

/* FILTER PARAMETERS */

// The string that determines what the filter does with the covariance on the
// state after filtering. The choices are as follows:
//     "yes" - Both position and velocity components of the covariance are
//             reset.
//     "posOnly" - Only the position components of the covariance are reset.
//     "no" - None of the components of the covariance are reset.
std::string COV_RESET = "yes";

// The initial covariance on the arm components of the state.
// page 20
mat prepareINITIAL_ARM_COV()
{
     /* mat ans = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
     double posCov = 1.0e-7;
     double velCov = 1.0e-7 / TIME_BIN;
     ans(0, 0) = posCov;
     ans(1, 1) = posCov;
     ans(2, 2) = posCov;
     ans(3, 3) = velCov;
     ans(4, 4) = velCov;
     ans(5, 5) = velCov;

     for(int i = 0; i < 3 * N_LAGS; i++)
       ans(3 + i, 3 + i) = velCov;

    return ans; */

    mat ans = zeros<mat>(6, 6);
         double posCov = 1.0e-7;
         double velCov = 1.0e-7 / TIME_BIN;
         ans(0, 0) = posCov;
         ans(1, 1) = posCov;
         ans(2, 2) = posCov;
         ans(3, 3) = velCov;
         ans(4, 4) = velCov;
         ans(5, 5) = velCov;

        return ans;
}

mat INITIAL_ARM_COV = prepareINITIAL_ARM_COV();

// The initial covariance on the channel parameters, in V^2, as components of
// the state.
double channelCov = 1.0e-2;
mat INITIAL_CHANNEL_COV = channelCov * eye<mat>(3 * N_LAGS, 3 * N_LAGS);

// The variance of the signal at each channel. The numbers here are made up, but
// in any actual experiment a baseline reading of the variance should be taken
// at each channel and this value should be modified.
double baseVariance = 3.0e3;
mat CHANNEL_VARIANCES = baseVariance * ones<mat>(N_CHANNELS, 1);

// The system evolution matrix that describes how the channel parameters are
// expected to evolve over time.
mat F_CHANNEL = eye<mat>(3 * N_LAGS, 3 * N_LAGS);

// The increment covariance matrix that decribes how the channel parameters are
// expected to be affected by Gaussian noise in each time step.
mat Q_CHANNEL = zeros<mat>(3 * N_LAGS, 3 * N_LAGS);

// The system evolution matrix that decribes how the kinematic components are
// expected to evolve over time.
mat prepareF_ARM_UNDIRECTED()
{
    /* mat ans = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
    ans(0, 0) = 1;
    ans(1, 1) = 1;
    ans(2, 2) = 1;
    ans(3, 3) = 1;
    ans(4, 4) = 1;
    ans(5, 5) = 1;
    ans(0, 3) = TIME_BIN;
    ans(1, 4) = TIME_BIN;
    ans(2, 5) = TIME_BIN;
    ans.diag(-3) = ones<vec>(3 * N_LAGS);
    ans(3, 0) = 0;
    ans(4, 1) = 0;
    ans(5, 2) = 0;

    return ans; */

    mat ans = eye<mat>(6, 6);
    ans(0, 3) = TIME_BIN;
    ans(1, 4) = TIME_BIN;
    ans(2, 5) = TIME_BIN;

    return ans;
}

mat F_ARM_UNDIRECTED = prepareF_ARM_UNDIRECTED();

// The increment covariance matrix that describes how the kinematics of the arm
// are expected to be affected by Gaussian noise in each time step.
mat prepareQ_ARM_UNDIRECTED()
{
    /* double velInc = 1.0e-5 / TIME_BIN;
    double historyInc = 1.0e-9 / TIME_BIN;

    mat ans = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
    ans(3, 3) = velInc;
    ans(4, 4) = velInc;
    ans(5, 5) = velInc;

    for(int i = 0; i < 3 * N_LAGS; i++)
        ans(3 + i, 3 + i) = historyInc;

    return ans; */

    double velInc = 1.0e-4 / TIME_BIN;

    mat ans = zeros<mat>(6, 6);
    ans(3, 3) = velInc;
    ans(4, 4) = velInc;
    ans(5, 5) = velInc;

    return ans;
}

mat Q_ARM_UNDIRECTED = prepareQ_ARM_UNDIRECTED();

// This is the number of time steps that the reach is expected to take.
int REACH_TIME_STEPS = (int)(1 / TIME_BIN);

// This is the spatial location of the target of the reach, with coordinates
// given in meters. The target is constant, but by translating what is displayed
// to the subject one can achieve any combination of arm start position and
// target location.
mat REACH_TARGET_POS = zeros<mat>(3, 1);

// This is what the velocity of the hand is expected to be at the end of the
// reach.
mat REACH_TARGET_VEL = zeros<mat>(3, 1);

// This is the target of the reach, position, velocity, and velocity history
// combined into the mean of a distribution on the state of the filter.
// mat REACH_TARGET = zeros<mat>(3 + 3 * N_LAGS, 1);
mat REACH_TARGET = zeros<mat>(6, 1);

// This is the assumed covariance on the above state.
mat prepareREACH_TARGET_COVARIANCE()
{
    /* double finalPosCov = 1.0e-6;
    double finalVelCov = 1.0e-8;
    double velHistoryCov = 1.0;

    mat ans = velHistoryCov * \
        eye<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
    ans(0, 0) = finalPosCov;
    ans(1, 1) = finalPosCov;
    ans(2, 2) = finalPosCov;
    ans(3, 3) = finalVelCov;
    ans(4, 4) = finalVelCov;
    ans(5, 5) = finalVelCov; */

    double finalPosCov = 1.0e-6;
    double finalVelCov = 1.0e-8;

    mat ans = zeros<mat>(6, 6);
    ans(0, 0) = finalPosCov;
    ans(1, 1) = finalPosCov;
    ans(2, 2) = finalPosCov;
    ans(3, 3) = finalVelCov;
    ans(4, 4) = finalVelCov;
    ans(5, 5) = finalVelCov;

    return ans;
}

mat REACH_TARGET_COVARIANCE = prepareREACH_TARGET_COVARIANCE();

/* CHANNEL PARAMETER GENERATION PARAMETERS */
// Right now it just returns 0 for all parameters. This can be revised when we
// have a better idea of what reasonable values for the parameters would be.

/* GRAPHING PARAMETERS */
// The channel for which we are graphing parameter values vs time step over all
// trials.
int DISPLAYED_CHANNEL_NUM = 4;
