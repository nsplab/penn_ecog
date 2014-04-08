#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <armadillo>
#include <string>

/* SYSTEM PARAMETERS */

// Number of EEG channels that are being used to obtain signals.
static int N_CHANNELS = 24;

// Number of velocity lag terms that are present in the signal model, including
// the one from the present time step.
static int N_LAGS = 10;

// Length of each time bin, in seconds. 
static double TIME_BIN = 0.01;

/* TRAINING RUN PARAMETERS */

// The number of trials in each training run.
static int N_TRIALS = 11;

// An array of booleans that determines which trials are test trials and which
// are training trials. If TEST_TRIALS[i] is true, then trial 'i' is a test
// trial, and if TEST_TRIALS[i] is false then trial 'i' is a training trial.
static bool TEST_TRIALS[N_TRIALS];

int everyNthTrial = 5;
for(int i=0; i<N_TRIALS; i++)
{
    TEST_TRIALS[i] = (i % everyNthTrial == 0);
}

// This is the number of seconds that the simulation should pause before the
// start of a new trial so that the user can see where his rendered hand is in
// relation to the target.
static int TIME_BETWEEN_TRIALS = 2;

/* TRIAL PARAMETERS */
// This is the distance, in meters, that the hand starts from the center of the
// target.
static double START_MAG = 0.2;

// This is the radius of the spherical target, in meters.
static double TARGET_RADIUS = 0.05;

// This is the maximum number of time steps that the trial will go on for.
static int MAX_TIME_STEPS = (int) (3 / TIME_BIN);

// This is the number of time steps that the hand must remain with TARGET_RADIUS
// of the target for the trial to end in success.
static int HOLD_TIME_STEPS = (int) (0.5 / TIME_BIN);

/* FILTER PARAMETERS */

// The string that determines what the filter does with the covariance on the
// state after filtering. The choices are as follows:
//     "posOnly" - Only the position components of the covariance are reset.
//     "no" - None of the components of the covariance are reset.
static string COV_RESET = "posOnly";

// The initial covariance on the arm components of the state.
static mat INITIAL_ARM_COV = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
double posCov = 1.0e-7;
double velCov = 1.0e-7 / TIME_BIN;
INITIAL_ARM_COV(0, 0) = posCov;
INITIAL_ARM_COV(1, 1) = posCov;
INITIAL_ARM_COV(2, 2) = posCov;
INITIAL_ARM_COV(3, 3) = velCov;
INITIAL_ARM_COV(4, 4) = velCov;
INITIAL_ARM_COV(5, 5) = velCov;

// The initial covariance on the channel parameters, in V^2, as components of
// the state.
double channelCov = 1.0e-10; 
static mat INITIAL_CHANNEL_COV = channelCov * eye<mat>(3 * N_LAGS, 3 * N_LAGS);

// The variance of the signal at each channel. The numbers here are made up, but
// in any actual experiment a baseline reading of the variance should be taken
// at each channel and this value should be modified.
double baseVariance = 1.0;
static mat CHANNEL_VARIANCES = baseVariance * ones<mat>(N_CHANNELS, 1);

// The system evolution matrix that describes how the channel parameters are
// expected to evolve over time.
static mat F_CHANNEL = eye<mat>(3 * N_LAGS, 3 * N_LAGS);

// The increment covariance matrix that decribes how the channel parameters are
// expected to be affected by Gaussian noise in each time step.
static mat Q_CHANNEL = zeros<mat>(3 * N_LAGS, 3 * N_LAGS);

// The system evolution matrix that decribes how the kinematic components are
// expected to evolve over time.
static mat F_ARM_UNDIRECTED = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
F_ARM_UNDIRECTED(0, 0) = 1;
F_ARM_UNDIRECTED(1, 1) = 1;
F_ARM_UNDIRECTED(2, 2) = 1;
F_ARM_UNDIRECTED(3, 3) = 1;
F_ARM_UNDIRECTED(4, 4) = 1;
F_ARM_UNDIRECTED(5, 5) = 1;
F_ARM_UNDIRECTED(0, 3) = TIME_BIN;
F_ARM_UNDIRECTED(1, 4) = TIME_BIN;
F_ARM_UNDIRECTED(2, 5) = TIME_BIN;
F_ARM_UNDIRECTED.diag(-3) = ones<vec>(3 * N_LAGS);
F_ARM_UNDIRECTED(3, 0) = 0;
F_ARM_UNDIRECTED(4, 1) = 0;
F_ARM_UNDIRECTED(5, 2) = 0;

// The increment covariance matrix that describes how the kinematics of the arm
// are expected to be affected by Gaussian noise in each time step.
double velInc = 1.0e-3;
static mat Q_ARM_UNDIRECTED = zeros<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
Q_ARM_UNDIRECTED(3, 3) = velInc;
Q_ARM_UNDIRECTED(4, 4) = velInc;
Q_ARM_UNDIRECTED(5, 5) = velInc;

// This is the number of time steps that the reach is expected to take.
static int REACH_TIME_STEPS = (int)(2 / TIME_BIN);

// This is the spatial location of the target of the reach, with coordinates
// given in meters. The target is constant, but by translating what is displayed
// to the subject one can achieve any combination of arm start position and
// target location.
static mat REACH_TARGET_POS = zeros<mat>(3, 1);

// This is what the velocity of the hand is expected to be at the end of the
// reach.
static mat REACH_TARGET_VEL = zeros<mat>(3, 1);

// This is the target of the reach, position, velocity, and velocity history
// combined into the mean of a distribution on the state of the filter.
static mat REACH_TARGET = zeros<mat>(3 + 3 * N_LAGS, 1);

// This is the assumed covariance on the above state.
double finalPosCov = 1.0e-6;
double finalVelCov = 1.0e-8;
double velHistoryCov = 1.0;
static mat REACH_TARGET_COVARIANCE = velHistoryCov * \
    eye<mat>(3 + 3 * N_LAGS, 3 + 3 * N_LAGS);
REACH_TARGET_COVARIANCE(0, 0) = finalPosCov;
REACH_TARGET_COVARIANCE(1, 1) = finalPosCov;
REACH_TARGET_COVARIANCE(2, 2) = finalPosCov;
REACH_TARGET_COVARIANCE(3, 3) = finalVelCov;
REACH_TARGET_COVARIANCE(4, 4) = finalVelCov;
REACH_TARGET_COVARIANCE(5, 5) = finalVelCov;

/* CHANNEL PARAMETER GENERATION PARAMETERS */
// Right now it just returns 0 for all parameters. This can be revised when we
// have a better idea of what reasonable values for the parameters would be.
#endif