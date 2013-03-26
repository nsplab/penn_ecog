#include "naive_filter.h"
#include <iostream>

using namespace std;

NaiveFilter::NaiveFilter(){

}

void NaiveFilter::Predict() {
  GrabFeatures();
}

void NaiveFilter::Update() {

}

void NaiveFilter::Run() {
  vector<float> hand_movement;
  for (;;) {
      Update();
      Predict();
      PublishHandMovement(hand_movement);
    }
}
