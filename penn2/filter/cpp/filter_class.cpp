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
    int hwm = 1;				//hwm - high water mark - determines buffer size for
    //data passed through ZMQ. hwm = 1 makes the ZMQ buffer
    //size = 1. This means that if no module has accessed a
    //value written through ZMQ, new values will be dropped
    //until any module reads the value
    features_subscriber_.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));

    features_subscriber_.connect("ipc:///tmp/features.pipe");
    features_subscriber_.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    supervisor_.connect("ipc:///tmp/supervisor.pipe");
    // assume target is in 3d
    target_.resize(3);
    handPos_.resize(3, 0.0);
}

void FilterClass::GrabFeatures() {                                                          // receives new features via zmq from the feature_extraction module
  // receive data from feature extractor
  zmq::message_t features_msg;                                                              // define a new object of the message_t class in the zmq namespace
  features_subscriber_.recv(&features_msg);                                                 // wait for the next new message in features_msg (this is "blocking" request;
                                                                                            // it won't proceed until a new value is detected by recv())
                                                                                            // features_subscriber_ is initialized in the FilterClass constructor to connect to "ipc:///tmp/features.pipe"

  // extract timestamp, # features, and feature values
  memcpy(&featureTimestamp_, features_msg.data(), sizeof(size_t));                          // copy sizeof(size_t) bytes from feature_msg.data() into featureTimestamp_, which is an unsigned integer of data type size_t
  cout<<"timestamp: "<<featureTimestamp_<<endl;                                             // (debug code) print the timestamp to the console

  size_t vec_size = (features_msg.size() - sizeof(size_t)) / sizeof(float);                 // calculate how long (in # of floats) the feature message is, minus the size of the featureTimestamp that was already extracted

  cout<<"vec size: "<<vec_size<<endl;                                                       // (debug code) print the number of floats left in the message to console

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

void FilterClass::SendHandPosGetState(const vector<float>& hand_movement) {                 // purpose: send updated hand position estimate from filter, and receive resulting hand position from supervisor.
                                                                                            // input:    hand_movement (passed by reference)
                                                                                            // function: uses ZMQ to send command from filter to supervisor on hand placement
                                                                                            //           receives the hand position set by the supervisor, which may be different (such as when velocity is integrated)
    stringstream message;                                                                   // message is a stringstream that will contain featureTimestamp and hand_movement data
    message<<featureTimestamp_<<" ";                                                        // pack the featureTimestamp into message
    for (vector<float>::const_iterator it=hand_movement.begin();                            // iterate over the array values in hand_movement and copy them to the message stringstream
         it<hand_movement.end(); it++) {input
      message<<*it<<" ";
    }
    message<<endl;                                                                          // complete the message stringstream with an endl character.
    zmq::message_t zmq_message(message.str().length());                                     // create a zmq message_t to send the message to the supervisor
    memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());     // insert the message stringstream into the message_t object zmq_message
    supervisor_.send(zmq_message);                                                          // use the supervisor ipc connection (established in constructor for FilterClass) to send the message
    cout<<"message sent from filter to supervisor"<<endl;                                   // (debug code) print to the console that the message was sent from the filter to the supervisor

                                                                                            // receive data from supervisor that contains the updated state of the hand in the virtual environment
    zmq::message_t supervisor_msg;                                                          // define a message_t to receive the supervisor message
    supervisor_.recv(&supervisor_msg);                                                      // use the supervisor ipc connection (established in the constructor for FilterClass) to receive the message (blocking)

    string supervisor_msg_str;                                                              // create a string to receive the supervisor message
    supervisor_msg_str.resize(supervisor_msg.size(),'\0');                                  // define the string size based on the size of the supervisor message
    supervisor_msg_str.assign((char *)supervisor_msg.data(),supervisor_msg.size());         // convert the received supervisor message into c++ string/sstream
    cout<<"feat_str "<<supervisor_msg_str<<endl;                                            // (debug code) print to the console what the supervisor string was
    //replace(feat_str.begin(), feat_str.end(), ',', ' ');
    stringstream supervisor_msg_ss(supervisor_msg_str);                                     //convert the supervisor_msg_str into a stringstream for convenient parsing

    // extract target position, hand position, trial ID, mode (training/testing)
    // and attending value from supervisor's message
    supervisor_msg_ss >> target_[0];supervisor_msg_ss >> target_[1];supervisor_msg_ss >> target_[2];             //without loss of generality, store the target position in 3-dim space (even for 2D or 1D mode)

    supervisor_msg_ss >> handPos_[0];supervisor_msg_ss >> handPos_[1];supervisor_msg_ss >> handPos_[2];          //without loss of generality, store the hand position in 3-dim space (even for 2D or 1D mode)

    supervisor_msg_ss >> trial_id;

    cout<<"trial_id "<<trial_id<<endl;

    int mode;
    supervisor_msg_ss >> mode;
    mode_ = (TrialMode)mode;

    supervisor_msg_ss >> attending_;
}

void FilterClass::Simulate(vector<float> features, size_t trial, vector<float> target, std::vector<float> initHandPosition) {
    features_ = features;
    trial_id = trial;
    target_ = target;
    handPos_ = initHandPosition;
}
