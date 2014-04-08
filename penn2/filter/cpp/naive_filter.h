#ifndef NAIVE_FILTER_H
#define NAIVE_FILTER_H

#include "filter_class.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>

#include <vector>

#include <fstream>

class NaiveFilter : public FilterClass {
public:
  typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::right> > > accumulator_t_right;
  typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::left> > > accumulator_t_left;

  NaiveFilter(float featureRate);
  void Update();
  void Predict();
  void Run();
  void runGUI(float& alpha, float& scale, bool& updated);
  void updateEwmaVariances();
  void kill();
private:
  FILE* eFile;
  float featureRate_;
  std::vector<float> means;
  std::vector<float> variances;
  std::vector<float> ewmaVariances;
  std::vector<float> ewmaValues;
  float gamma_;
  float scale_;
  float q5_;
  float q95_;
  bool quit;
  bool updated_;
};

#endif // NAIVE_FILTER_H
