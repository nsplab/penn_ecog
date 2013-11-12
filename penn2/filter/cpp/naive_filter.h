#ifndef NAIVE_FILTER_H
#define NAIVE_FILTER_H

#include "filter_class.h"

class NaiveFilter : public FilterClass {
public:
  NaiveFilter(float featureRate);
  void Update();
  void Predict();
  void Run();
private:
  float featureRate_;
};

#endif // NAIVE_FILTER_H
