#include "naive_filter.h"
#include <iostream>

using namespace std;

NaiveFilter::NaiveFilter(){

}

void NaiveFilter::Predict() {
  // get features in features_
  GrabFeatures();
  // process features
  for (size_t i=0; i<features_.size(); i++) {
      features_[i] *= 0.5;
  }
}

void NaiveFilter::Update() {
    float baseline = 50.0;
    float scale = 2.0;//30.0;
    handPos_[0] += (features_[0] - baseline) / scale;
    handPos_[1] += (features_[1] - baseline) / scale;
    handPos_[2] += (features_[2] - baseline) / scale;
}

void NaiveFilter::Run() {
  for (;;) {
      SendHandPosGetState(handPos_);
      Predict();
      Update();
    }
}
