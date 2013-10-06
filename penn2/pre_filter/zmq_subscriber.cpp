#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>

#include <zmq.hpp>

using namespace std;
using namespace zmq;

int main(int argc, char *argv[])
{
  const char *input = "ipc:///tmp/features.pipe";

  context_t context_(1);
  socket_t subscriber(context_, ZMQ_SUB);
  subscriber.connect(input);
  subscriber.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);

  while (true) {
    // receive data from stft
    zmq::message_t frequency_msg;
    subscriber.recv(&frequency_msg);
    string feat_str(((char *) frequency_msg.data()));
    cout << feat_str << "\n";
  }
}

