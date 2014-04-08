#ifndef FILTER_CLASS_H
#define FILTER_CLASS_H

#include <vector>
#include <zmq.hpp>

enum TrialMode {TRAINING, TESTING};

class FilterClass {
public:
  FilterClass();
  virtual ~FilterClass() {};

  void GrabFeatures();                                              // receive features from the feature extraction module
  void SendHandPosGetState(const std::vector<float>& hand_movement);// receive virtual hand state from supervisor: hand position, mode (training/testing), target position

                                                                    // classes derived from FilterClass should implement these two methods
  virtual void Update() = 0;                                        // update step of a recursive filter (named with Bayesian filters in mind)
  virtual void Predict() = 0;                                       // prediction step of a recursive filter (named with Bayesian filters in mind)
                                                                    // setting the function to 0 tells the compiler there is no implementation for these functions
                                                                    // this prevents the compiler from creating objects of this base class
                                                                    // we're expecting to create derived classes that implement these functions (like naive_filter)
  virtual void Run() = 0;                                           // virtual function that should contain the main loop of the filter,
                                                                    // using dynamic binding at runtime we select the filter we want to use

  // auxiliary function to test the filter
  void Simulate(std::vector<float> features, size_t trial, std::vector<float> target, std::vector<float> initHandPosition);
  std::vector<float> GetHandState() {return handState_;}            // returns the handState_
protected:
  std::vector<float> features_;                                     // feature vector at the current point in time, typically power at various frequencies/channels
  std::vector<float> handPos_;                                      // update-step-based hand position intended to be sent to the virtual environment at the current point in time
  std::vector<float> handState_;                                    // hand state variable - could be used to record position and velocity

  TrialMode mode_;                                                  // trial mode - training (adaptive version of JointRSE) or testing (static version of JointRSE) mode
  float attending_;                                                 // whether the subject is attending to the task; future versions of the code will use head tracking, etc. to check if the
                                                                    // subject is attending.
  size_t trial_id;                                                  // trial ID sent by supervisor; should be sent back to it
  // target position sent by supervisor
  std::vector<float> target_;
  size_t featureTimestamp_;                                         //timestamping for each feature is based on a counter rather than system time. the size_t data type is an unsigned integer value.
  size_t dim_;
private:
  static zmq::context_t context_;                                   //
  static zmq::socket_t supervisor_;                                 // zmq ipc channel for messages broadcast by the supervisor module
  static zmq::socket_t features_subscriber_;                        // zmq ipc channel for messages broadcast by the feature_extraction module
};

#endif // FILTER_CLASS_H
