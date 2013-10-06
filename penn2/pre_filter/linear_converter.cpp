#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>

#include <zmq.hpp>

using namespace std;
using namespace zmq;

int main(int argc, char *argv[])
{
  const char *filename = "matrix";
  const char *output = "ipc:///tmp/features.pipe";
  const char *input = "ipc:///tmp/frequencies.pipe";

  FILE *file = fopen(filename, "r");

  int rows, cols;

  fscanf(file, "%d %d\n", &rows, &cols);

  float **matrix = new float *[rows];
  for (int i = 0; i < rows; i++) {
    matrix[i] = new float[cols];
    for (int j = 0; j < cols; j++) {
      fscanf(file, "%f", matrix[i] + j);
    }
  }

  // TODO: verify file is right size

  context_t context_(2);
  socket_t publisher(context_, ZMQ_PUB);
  socket_t subscriber(context_, ZMQ_SUB);

  publisher.bind(output);
  subscriber.connect(input);
  subscriber.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);

  float *ans = new float[rows];
  while (true) {
    for (int i = 0; i < rows; i++) {
      ans[i] = 0.0;
    }

    // receive data from stft
    zmq::message_t frequency_msg;
    subscriber.recv(&frequency_msg);

    // convert reveived data into c++ string/sstream
    string feat_str(((char *) frequency_msg.data()));
    replace(feat_str.begin(), feat_str.end(), ',', ' ');
    stringstream ss;
    ss.str(feat_str);

    // extract frequency input
    for (int i = 0; i < cols; i++) {
      float v;
      ss >> v;
      for (int j = 0; j < rows; j++) {
        ans[j] += matrix[j][i] * v;
      }
    }

    // generate output string
    stringstream message;
    for (int i = 0; i < rows; i++) {
      message << ans[i] << ",";
    }
    message<<endl;
    zmq::message_t zmq_message(message.str().length());
    memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
    publisher.send(zmq_message);
  }

  fclose(file);
}

