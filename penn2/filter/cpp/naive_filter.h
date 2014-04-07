#ifndef NAIVE_FILTER_H
#define NAIVE_FILTER_H

#include "filter_class.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>

#include <vector>

class NaiveFilter : public FilterClass {                        //this is the prototype for the NaiveFilter class; details are found in naive_filter.cpp
                                                                //NaiveFilter is the derived class, which inherits members from the base class called FilterClass
                                                                //members that you see below are in addition to the members present in FilterClass
                                                                //note that Update() and Predict() were described but not implemented in FilterClass; they are implemented in NaiveFilter
public:
  typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::right> > > accumulator_t_right;
  typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::tail_quantile<boost::accumulators::left> > > accumulator_t_left;

  NaiveFilter(float featureRate);
  void Update();
  void Predict();
  void Run();
  void runGUI(float& alpha, float& scale, bool& updated, float mean);
  void updateEwmaVariances();
private:
  float featureRate_;
  std::vector<float> means;
  std::vector<float> variances;
  std::vector<float> ewmaVariances;
  std::vector<float> ewmaValues;
  float gamma_;
  float scale_;
  float q5_;
  float q95_;
};

#endif // NAIVE_FILTER_H
