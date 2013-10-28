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
      features_[i] *= 1.2;
  }
}

void NaiveFilter::Update() {
    handMovement_[0] = features_[0];
    handMovement_[1] = features_[1];
    handMovement_[2] = features_[2];
}

void NaiveFilter::Run() {
  for (;;) {
      Predict();
      Update();
      PublishHandMovement(handMovement_);
    }
}
