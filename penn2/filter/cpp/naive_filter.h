#ifndef NAIVE_FILTER_H
#define NAIVE_FILTER_H

#include "filter_class.h"

class NaiveFilter : public FilterClass {
public:
  NaiveFilter();
  void Update();
  void Predict();
  void Run();
};

#endif // NAIVE_FILTER_H
