#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <armadillo>
#include <string>

using namespace arma;

/* SYSTEM PARAMETERS */

// Number of EEG channels that are being used to obtain signals.
extern int N_CHANNELS;

// Number of velocity lag terms that are present in the signal model, including
// the one from the present time step.
extern int N_LAGS;

// Length of each time bin, in seconds. 
extern double TIME_BIN;

// The number of baseline samples that are taken to compute the mean and variance
// of the signal at each channel.
extern int N_BASELINE_SAMPLES;

/* TRAINING RUN PARAMETERS */

// The number of trials in each training run.
extern int N_TRIALS;

// An array of booleans that determines which trials are test trials and which
// are training trials. If TEST_TRIALS[i] is true, then trial 'i' is a test
// trial, and if TEST_TRIALS[i] is false then trial 'i' is a training trial.
extern bool* TEST_TRIALS;

// The number of test and training trials in each training run. Derived from
// TEST_TRIALS.
extern int N_TEST_TRIALS;
extern int N_TRAIN_TRIALS;

// This is the number of seconds that the simulation should pause before the
// start of a new trial so that the user can see where his rendered hand is in
// relation to the target.
extern int TIME_BETWEEN_TRIALS;

/* TRIAL PARAMETERS */
// This is the distance, in meters, that the hand starts from the center of the
// target.
extern double START_MAG;

// This is the radius of the spherical target, in meters.
extern double TARGET_RADIUS;

// This is the maximum number of time steps that the trial will go on for.
extern int MAX_TIME_STEPS;

// This is the number of time steps that the hand must remain with TARGET_RADIUS
// of the target for the trial to end in success.
extern int HOLD_TIME_STEPS;

/* FILTER PARAMETERS */

// The string that determines what the filter does with the covariance on the
// state after filtering. The choices are as follows:
//     "posOnly" - Only the position components of the covariance are reset.
//     "no" - None of the components of the covariance are reset.
extern std::string COV_RESET;

// The initial covariance on the arm components of the state.
extern mat INITIAL_ARM_COV;

// The initial covariance on the channel parameters, in V^2, as components of
// the state.
extern mat INITIAL_CHANNEL_COV;

// The variance of the signal at each channel. The numbers here are made up, but
// in any actual experiment a baseline reading of the variance should be taken
// at each channel and this value should be modified.
extern mat CHANNEL_VARIANCES;

// The system evolution matrix that describes how the channel parameters are
// expected to evolve over time.
extern mat F_CHANNEL;

// The increment covariance matrix that decribes how the channel parameters are
// expected to be affected by Gaussian noise in each time step.
extern mat Q_CHANNEL;

// The system evolution matrix that decribes how the kinematic components are
// expected to evolve over time.
extern mat F_ARM_UNDIRECTED;

// The increment covariance matrix that describes how the kinematics of the arm
// are expected to be affected by Gaussian noise in each time step.
extern mat Q_ARM_UNDIRECTED;

// This is the number of time steps that the reach is expected to take.
extern int REACH_TIME_STEPS;

// This is the spatial location of the target of the reach, with coordinates
// given in meters. The target is constant, but by translating what is displayed
// to the subject one can achieve any combination of arm start position and
// target location.
extern mat REACH_TARGET_POS;

// This is what the velocity of the hand is expected to be at the end of the
// reach.
extern mat REACH_TARGET_VEL;

// This is the target of the reach, position, velocity, and velocity history
// combined into the mean of a distribution on the state of the filter.
extern mat REACH_TARGET;

// This is the assumed covariance on the above state.
extern mat REACH_TARGET_COVARIANCE;

/* CHANNEL PARAMETER GENERATION PARAMETERS */
// Right now it just returns 0 for all parameters. This can be revised when we
// have a better idea of what reasonable values for the parameters would be.

/* GRAPHING PARAMETERS */
// The channel for which we are graphing parameter values vs time step over all
// trials.
extern int DISPLAYED_CHANNEL_NUM;
#endif
