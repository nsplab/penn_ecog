#include <stdlib.h>

#include <zmq.hpp>
#include <sstream>

#include "fft.h"

using namespace std;
using namespace zmq;

int main(int argc, char *argv[])
{
  const char *output = "ipc:///tmp/frequencies.pipe";
  const char *input = "ipc:///tmp/ecog.pipe";

  size_t numChannels = 1;
  Fft<double>::windowFunc window = Fft<double>::HAMMING;
  size_t sampling_frq = 25000; // Hz (this value is entirely ignored)
  size_t dft_points = 6250; // points
  size_t fft_offset = 2500;

  context_t context_(2);
  socket_t publisher(context_, ZMQ_PUB);
  socket_t subscriber(context_, ZMQ_SUB);

  publisher.bind(output);
  subscriber.connect(input);
  subscriber.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);


  Fft<double> fft(dft_points, window, sampling_frq, numChannels);

  vector<vector<double> > powers(numChannels);
  vector<vector<double> > phases(numChannels);
  for (size_t i=0; i<numChannels; i++) {
    powers[i].resize(dft_points/2+1);
    phases[i].resize(dft_points/2+1);
  }

  vector<double> point(numChannels);

  int iteration = 0;
  while (true) {
    // receive data from ECoG
    zmq::message_t ecog_msg;
    subscriber.recv(&ecog_msg);

    // extract ECoG input
    for (int i = 0; i < numChannels; i++) {
      point[i] = ((float *) ecog_msg.data())[i];
    }

    fft.AddPoints(point);

    iteration++;
    if (iteration >= fft_offset) {
      if (fft.Process()) {
        fft.GetPower(powers);
        fft.GetPhase(phases);
      }
      else {
        cout << "Process failed.\n";
      }

      // generate output string
      stringstream message;
      for (int i = 0; i < numChannels; i++) {
        for (int j = 0; j < dft_points/2+1; j++) {
          message << powers[i][j] << " ";
        }
      }
      message<<endl;
      zmq::message_t zmq_message(message.str().length());
      memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
      publisher.send(zmq_message);
    }
  }
}

