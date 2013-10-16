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
  const char *output = "ipc:///tmp/frequencies.pipe";

  context_t context_(1);
  socket_t publisher(context_, ZMQ_PUB);
  publisher.bind(output);

  while (true) {
    // generate output string
    stringstream message;

    string temp;
    cin >> temp;

    message << temp;
    message << endl;

    zmq::message_t zmq_message(message.str().length());
    memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
    publisher.send(zmq_message);
  }
}

