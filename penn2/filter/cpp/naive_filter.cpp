#include "naive_filter.h"
#include <iostream>

using namespace std;

NaiveFilter::NaiveFilter(float featureRate){
  featureRate_ = featureRate;
}

void NaiveFilter::Predict() {
  // get features in features_
  GrabFeatures();
  // process features
  //for (size_t i=0; i<features_.size(); i++) {
  //    features_[i] *= 0.5;
  //}
}

void NaiveFilter::Update() {
    float baseline = 100.0;
    float scale = 2.0;
    float velx = (features_[0] - baseline) / scale * (1.0/featureRate_);
    float vely = (features_[1] - baseline) / scale * (1.0/featureRate_);
    float velz = (features_[2] - baseline) / scale * (1.0/featureRate_);
    cout<<"velx "<<velx<<endl;
    cout<<"vely "<<vely<<endl;
    cout<<"velz "<<velz<<endl;
    cout<<"tx "<<features_[0]<<endl;
    cout<<"ty "<<features_[1]<<endl;
    cout<<"tz "<<features_[2]<<endl;
    handPos_[0] += velx;
    handPos_[1] += vely;
    handPos_[2] += velz;
}

void NaiveFilter::Run() {
  for (;;) {
      SendHandPosGetState(handPos_);
      Predict();
      Update();
    }
}
