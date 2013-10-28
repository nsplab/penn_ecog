#ifndef FILTER_CLASS_H
#define FILTER_CLASS_H

#include <vector>
#include <zmq.hpp>

enum TrialMode {TRAINING, TESTING};

class FilterClass {
public:
  FilterClass();
  // receive features from the feature extraction module
  void GrabFeatures();
  // receive state from supervisor:
  // hand position, mode (training/testing), target position
  void GetState();
  // use ZMQ to publish hand movement at next cycle
  // sent values are added to current hand position by supervisor
  void PublishHandMovement(const std::vector<float>& hand_movement);
  // derived classes should implement these two methods
  virtual void Update() = 0;
  virtual void Predict() = 0;

  // auxiliary function to test the filter
  void Simulate(std::vector<float> features, size_t trial, std::vector<float> target, std::vector<float> initHandPosition);
  std::vector<float> GetHandState() {return handState_;}
protected:
  // feature vector
  std::vector<float> features_;
  // hand movement
  std::vector<float> handMovement_;
  // hand position
  std::vector<float> handPos_;
  // hand position
  std::vector<float> handState_;
  // training or testing mode
  TrialMode mode_;
  // trial ID sent by supervisor
  // should be sent back to it
  size_t trial_id;
  // target position sent by supervisor
  std::vector<float> target_;
  size_t featureTimestamp_;
private:
  static zmq::context_t context_;
  static zmq::socket_t publisher_;
  static zmq::socket_t supervisor_subscriber_;
  static zmq::socket_t features_subscriber_;
};

#endif // FILTER_CLASS_H
