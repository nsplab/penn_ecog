#ifndef NAIVE_FILTER_H
#define NAIVE_FILTER_H

#include "filter_class.h"

#include <vector>

class NaiveFilter : public FilterClass {
public:
  NaiveFilter(float featureRate);
  void Update();
  void Predict();
  void Run();
  void runGUI(float& alpha, float& scale, bool& updated);
  void updateEwmaVariances();
private:
  float featureRate_;
  std::vector<float> means;
  std::vector<float> variances;
  std::vector<float> ewmaVariances;
  std::vector<float> ewmaValues;
  float alpha_;
  float scale_;
};

#endif // NAIVE_FILTER_H
