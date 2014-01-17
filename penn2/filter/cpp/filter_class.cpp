#include "filter_class.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace zmq;

// ZMQ thread pool of 3
context_t FilterClass::context_(2);
socket_t FilterClass::supervisor_(context_, ZMQ_REQ);
socket_t FilterClass::features_subscriber_(context_, ZMQ_SUB);

FilterClass::FilterClass() {
  features_subscriber_.connect("ipc:///tmp/features.pipe");
  features_subscriber_.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
  supervisor_.connect("ipc:///tmp/supervisor.pipe");
  // assume target is in 3d
  target_.resize(3);
  handPos_.resize(3, 0.0);
}

void FilterClass::GrabFeatures() {
  // receive data from feature extractor
  zmq::message_t features_msg;
  features_subscriber_.recv(&features_msg);

  // extract timestamp, # features, and features
  memcpy(&featureTimestamp_, features_msg.data(), sizeof(size_t));

  cout<<"timestamp: "<<featureTimestamp_<<endl;

  size_t vec_size = (features_msg.size() - sizeof(size_t)) / sizeof(float);

  cout<<"vec size: "<<vec_size<<endl;

  if (features_.size() != vec_size) {
    features_.resize(vec_size);
  }
  memcpy(features_.data(), static_cast<size_t*>(features_msg.data())+1, vec_size * sizeof(float));

  cout<<"features: ";
  for (size_t i=0; i<vec_size; i++) {
      cout<<features_[i]<<" ";
  }
  cout<<endl;
}

void FilterClass::SendHandPosGetState(const vector<float>& hand_movement) {
    stringstream message;
    message<<featureTimestamp_<<" ";
    for (vector<float>::const_iterator it=hand_movement.begin();
         it<hand_movement.end(); it++) {
      message<<*it<<" ";
    }
    message<<endl;
    zmq::message_t zmq_message(message.str().length());
    memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
    supervisor_.send(zmq_message);
    cout<<"message sent"<<endl;

    // receive data from supervisor
    zmq::message_t supervisor_msg;
    supervisor_.recv(&supervisor_msg);
    // convert received data into c++ string/sstream
    string feat_str;
    feat_str.resize(supervisor_msg.size(),'\0');
    feat_str.assign((char *)supervisor_msg.data(),supervisor_msg.size());
    cout<<"feat_str "<<feat_str<<endl;
    //replace(feat_str.begin(), feat_str.end(), ',', ' ');
    stringstream ss(feat_str);

    // extract target position, hand position, trial ID, mode (training/testing)
    // and attending value from supervisor's message
    ss >> target_[0];ss >> target_[1];ss >> target_[2];

    ss >> handPos_[0];ss >> handPos_[1];ss >> handPos_[2];

    ss >> trial_id;

    cout<<"trial_id "<<trial_id<<endl;

    int mode;
    ss >> mode;
    mode_ = (TrialMode)mode;

    ss >> attending_;
}

void FilterClass::Simulate(vector<float> features, size_t trial, vector<float> target, std::vector<float> initHandPosition) {
    features_ = features;
    trial_id = trial;
    target_ = target;
    handPos_ = initHandPosition;
}
