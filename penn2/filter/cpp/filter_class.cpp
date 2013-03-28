#include "filter_class.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace zmq;

// ZMQ thread pool of 3
context_t FilterClass::context_(3);
socket_t FilterClass::publisher_(context_, ZMQ_PUB);
socket_t FilterClass::supervisor_subscriber_(context_, ZMQ_SUB);
socket_t FilterClass::features_subscriber_(context_, ZMQ_SUB);

FilterClass::FilterClass() {
  publisher_.bind("ipc:///tmp/handposition.pipe");
  features_subscriber_.connect("ipc:///tmp/features.pipe");
  features_subscriber_.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
  supervisor_subscriber_.connect("ipc:///tmp/supervisor.pipe");
  supervisor_subscriber_.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
  // the target is in 3 dimensions, TODO: fix me
  target_.resize(3);
  handMovement_.resize(3);
}

void FilterClass::GrabFeatures() {
  // receive data from feature extractor
  zmq::message_t features_msg;
  features_subscriber_.recv(&features_msg);
  // convert reveived data into c++ string/sstream
  string feat_str(((char *)features_msg.data()));
  replace(feat_str.begin(), feat_str.end(), ',', ' ');
  stringstream ss;
  ss.str(feat_str);

  // extract timestamp, # features, and features
  int timestamp;
  ss>>timestamp;
  int vec_size;
  ss>>vec_size;

  cout<<"timestamp: "<<timestamp<<endl;
  cout<<"vec size: "<<vec_size<<endl;

  features_.resize(vec_size);
  for (size_t i=0; i<vec_size; i++) {
      ss>>features_[i];
  }
  cout<<"features: ";
  for (size_t i=0; i<vec_size; i++) {
      cout<<features_[i]<<" ";
  }
  cout<<endl;
}

void FilterClass::GetState() {
  // receive data from supervisor
  zmq::message_t supervisor_msg;
  supervisor_subscriber_.recv(&supervisor_msg);
  // convert reveived data into c++ string/sstream
  string feat_str(((char *)supervisor_msg.data()));
  replace(feat_str.begin(), feat_str.end(), ',', ' ');
  stringstream ss;

  // extract trial ID, target position, and mode (training/testing)
  ss >> trial_id;

  ss >> target_[0];ss >> target_[1];ss >> target_[2];

  int mode;
  ss >> mode;
  mode_ = (TrialMode)mode;
}

void FilterClass::PublishHandMovement(const vector<float>& hand_movement) {
  stringstream message;
  for (vector<float>::const_iterator it=hand_movement.begin();
       it<hand_movement.end(); it++) {
    message<<*it<<",";
  }
  message<<endl;
  zmq::message_t zmq_message(message.str().length());
  memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
  publisher_.send(zmq_message);
}
