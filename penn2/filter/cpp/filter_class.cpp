#include "filter_class.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace zmq;                                                                        // this library is used for asynchronous communication (ipc) between supervisor, feature_extraction, and filter modules

                                                                                            // setting up ZMQ with a "thread pool" of 2 communication channels
context_t FilterClass::context_(2);                                                         // needed to establish the zmq channels. argument of 2 says there will be 2 communication channels
socket_t FilterClass::supervisor_(context_, ZMQ_REQ);                                       // ZMQ_REQ is the request side of a request-and-reply communication channel (two-way communication)
socket_t FilterClass::features_subscriber_(context_, ZMQ_SUB);                              // ZMQ_SUB is the subscribe side of a publish-subscribe communication channel (one-way communication

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FilterClass::FilterClass() {                                                                // purpose: constructor for FilterClass, establishes zmq ipc connections to features and supervisor
                                                                                            // inputs:  none

    int hwm = 1;                                                                            //hwm - high water mark - determines buffer size for
                                                                                            //data passed through ZMQ. hwm = 1 makes the ZMQ buffer
                                                                                            //size = 1. This means that if no module has accessed a
                                                                                            //value written through ZMQ, new values will be dropped
                                                                                            //until any module reads the value
    features_subscriber_.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));

    features_subscriber_.connect("ipc:///tmp/features.pipe");                               //connect to the ipc representing features, published by the feature_extraction module
    features_subscriber_.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    supervisor_.connect("ipc:///tmp/supervisor.pipe");                                      //connect to the ipc representing features, published by the supervisor module

    target_.resize(3);                                                                      // define the target position as a 3x1 vector without loss of generality (for 2d and 1d as well)
    handPos_.resize(3, 0.0);                                                                // define the hand position as a 3x1 vector without loss of generality (for 2d and 1d as well)
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void FilterClass::GrabFeatures() {                                                          // purpose: receives new features (neural observations, typically power at various frequencies/channels
                                                                                            //          via zmq from the feature_extraction module, and stores them in the protected filter variable features_
                                                                                            // input:   none

  zmq::message_t features_msg;                                                              // define a new object of the message_t class in the zmq namespace
  features_subscriber_.recv(&features_msg);                                                 // wait for the next new message in features_msg (this is "blocking" request;
                                                                                            // it won't proceed until a new value is detected by recv())
                                                                                            // features_subscriber_ is initialized in the FilterClass constructor to connect to "ipc:///tmp/features.pipe"

  // extract timestamp, # features, and feature values
  memcpy(&featureTimestamp_, features_msg.data(), sizeof(size_t));                          // copy sizeof(size_t) bytes from feature_msg.data() into featureTimestamp_, which is an unsigned integer of data type size_t
  cout<<"timestamp: "<<featureTimestamp_<<endl;                                             // (debug code) print the timestamp to the console

  size_t vec_size = (features_msg.size() - sizeof(size_t)) / sizeof(float);                 // calculate how long (in # of floats) the feature message is, minus the size of the featureTimestamp that was already extracted

  cout<<"vec size: "<<vec_size<<endl;                                                       // (debug code) print the number of floats left in the message to console

  if (features_.size() != vec_size) {                                                       // if the features_ vector size is different from the feature vector received just now,
    features_.resize(vec_size);                                                             // then resize the features_ vector in preparation to copy that data from the zmq message_t object features_msg
  }
  memcpy(features_.data(), static_cast<size_t*>(features_msg.data())+1,                     // store into features_ the latest value of the features stored in features_msg from zmq
                                                            vec_size * sizeof(float));

  cout<<"features: ";                                                                       // (debug code) write the feature values you've just received to the console
  for (size_t i=0; i<vec_size; i++) {                                                       // for each entry in the features_ vector
      cout<<features_[i]<<" ";                                                              // write that value to the console
  }
  cout<<endl;                                                                               // insert a new line after writing the features_ vector to console
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void FilterClass::SendHandPosGetState(const vector<float>& hand_movement) {                 // purpose: send updated hand position estimate from filter, and receive resulting hand position from supervisor.
                                                                                            // input:    hand_movement (passed by reference)
                                                                                            // function: uses ZMQ to send command from filter to supervisor on hand placement
                                                                                            //           receives the hand position set by the supervisor, which may be different (such as when velocity is integrated)

                                                                                            //*****************************************************************************************
                                                                                            // send filter update estimate to supervisor for placement of virtual hand
                                                                                            //*****************************************************************************************
    stringstream message;                                                                   // message is a stringstream that will contain featureTimestamp and hand_movement data
    message<<featureTimestamp_<<" ";                                                        // pack the featureTimestamp into message
    for (vector<float>::const_iterator it=hand_movement.begin();                            // iterate over the array values in hand_movement and copy them to the message stringstream
        it<hand_movement.end(); it++) {
        message<<*it<<" ";
    }
    message<<features_.size()<<" ";
    message<<parameters_.size()<<" ";
    for (vector<float>::const_iterator it=features_.begin();                            // iterate over the array values in features_ and copy them to the message stringstream
        it<features_.end(); it++) {
        message<<*it<<" ";
    }
    for (vector<float>::const_iterator it=parameters_.begin();                            // iterate over the array values in parameters_ and copy them to the message stringstream
        it<parameters_.end(); it++) {
        message<<*it<<" ";
    }

    message<<endl;                                                                          // complete the message stringstream with an endl character.
    zmq::message_t zmq_message(message.str().length());                                     // create a zmq message_t to send the message to the supervisor
    memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());     // insert the message stringstream into the message_t object zmq_message
    supervisor_.send(zmq_message);                                                          // use the supervisor ipc connection (established in constructor for FilterClass) to send the message
    cout<<"message sent from filter to supervisor"<<endl;                                   // (debug code) print to the console that the message was sent from the filter to the supervisor

                                                                                            //*****************************************************************************************
                                                                                            // receive data from supervisor that contains critical information, including
                                                                                            // updated state of the hand in the virtual environment, training vs. testing
                                                                                            // trial type requested by the supervisor, and subject's state of attentiveness (attending_)
                                                                                            //*****************************************************************************************
    zmq::message_t supervisor_msg;                                                          // define a message_t to receive the supervisor message
    supervisor_.recv(&supervisor_msg);                                                      // use the supervisor ipc connection (established in the constructor for FilterClass) to receive the message (blocking)

    string supervisor_msg_str;                                                              // create a string to receive the supervisor message
    supervisor_msg_str.resize(supervisor_msg.size(),'\0');                                  // define the string size based on the size of the supervisor message
    supervisor_msg_str.assign((char *)supervisor_msg.data(),supervisor_msg.size());         // convert the received supervisor message into c++ string/sstream
    cout<<"feat_str "<<supervisor_msg_str<<endl;                                            // (debug code) print to the console what the supervisor string was
    //replace(feat_str.begin(), feat_str.end(), ',', ' ');
    stringstream supervisor_msg_ss(supervisor_msg_str);                                     // convert the supervisor_msg_str into a stringstream for convenient parsing

                                                                                            // extract target position, hand position, trial ID, mode (training/testing)
                                                                                            // and subject attending state from supervisor's message

                                                                                            // without loss of generality, store the target position in 3-dim space (even for 2D or 1D mode)
    supervisor_msg_ss >> target_[0];                                                        // target x position
    supervisor_msg_ss >> target_[1];                                                        // target y position
    supervisor_msg_ss >> target_[2];                                                        // target z position
                                                                                            // without loss of generality, store the hand position in 3-dim space (even for 2D or 1D mode)
    supervisor_msg_ss >> handPos_[0];                                                       // hand x position in virtual environment, following command from filter to supervisor based on update filter estimate
    supervisor_msg_ss >> handPos_[1];                                                       // hand y position in virtual environment, following command from filter to supervisor based on update filter estimate
    supervisor_msg_ss >> handPos_[2];                                                       // hand z position in virtual environment, following command from filter to supervisor based on update filter estimate
    supervisor_msg_ss >> trial_id;                                                          // ?? trial_id gives trial number?
    cout<<"trial_id "<<trial_id<<endl;                                                      // (debug code) print the trial id to the console

    int mode;                                                                               // define an integer to store the filter mode requested by the supervisor (0 - training, 1 - testing)
    supervisor_msg_ss >> mode;                                                              // store the requested mode from the supervisor's message
    mode_ = (TrialMode)mode;                                                                // change the mode into a variable of type TrialMode
    supervisor_msg_ss >> attending_;                                                        // store the subject's attentiveness state determined by the supervisor
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void FilterClass::Simulate(vector<float> features, size_t trial,                            // purpose: ??
                           vector<float> target, std::vector<float> initHandPosition) {     // input:   features- feature vector, trial - trial number, target - position in 3-dim space (even for 2D or 1D mode)
                                                                                            //          initHandPosition - initial hand position in 3-dim space (even for 2D or 1D mode)
                                                                                            // function: assigns values to these protected variables within the class

    features_ = features;                                                                   // assign the protected FilterClass variable features_
    trial_id = trial;                                                                       // assign the protected FilterClass variable trial
    target_ = target;                                                                       // assign the protected FilterClass variable target
    handPos_ = initHandPosition;                                                            // assign the protected FilterClass variable initHandPosition
}
