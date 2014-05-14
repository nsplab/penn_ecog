/*! \file naive_filter.cpp
      \brief Implements the static filter
*/

#include "moving_average_filter.h"
#include <iostream>
#include <thread>
#include <fstream>

#include <Fl/Fl.H>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <FL/Fl_Output.H>

using namespace std;                                                                                //allows you to write cout << instead of std:cout <<


MovingAverageFilter::MovingAverageFilter(float featureRate){
  featureRate_ = featureRate;
  gamma_ = 1.0;
  scale_ = 1000.0;
  q5_ = 0.0;
  q95_ = 0.0;
  quit = false;
  updated_ = false;
  time_t rawtime;
  time(&rawtime);
  char nameBuffer[24];
  tm * ptm = localtime(&rawtime);
  strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
  string imageFilename = string("filter_data_")+string(nameBuffer);
  eFile = fopen(imageFilename.c_str(), "wb");
}

MovingAverageFilter::~MovingAverageFilter() {

}

void MovingAverageFilter::kill() {

}

void MovingAverageFilter::Predict() {
  // get features in features_
  GrabFeatures();
  // process features
  //for (size_t i=0; i<features_.size(); i++) {
  //    features_[i] *= 0.5;
  //}
}

void MovingAverageFilter::Update() {
    /*float baseline = 100.0;
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
    handPos_[2] += velz;*/

    //ewmaValues[0] = alpha_ * (features_[0] - means[0]) + (1.0 - alpha_) * ewmaValues[0];

    //double velx = double(features_[0] - means[0]) / (3.0 * sqrt(ewmaVariances[0]));
    float scaleX = scale_;
    float meanX = 39.7484;

    if (features_[0] < (3.0 * sqrt(ewmaVariances[0])) )  {
        if (features_[0] > (-3.0 * sqrt(ewmaVariances[0])) ) {
            features_[0] = 0.0;
        }
    }

    ewmaValues[0] = (1.0 - gamma_) * ewmaValues[0] + gamma_ * double(features_[0] - means[0]);

    double velx = scaleX * ewmaValues[0]; //double(features_[0] - means[0]) * scaleX;
    double vely = 0; //double(features_[1] - means[1]) / (3.0 * sqrt(ewmaVariances[1]));
    double velz = 0; //double(features_[2] - means[2]) / (3.0 * sqrt(ewmaVariances[2]));

    // safty zone
    /*if (ewmaValues[0] > q5_) {
        if (ewmaValues[0] < q95_) {
            velx = 0.0;
        }
    }*/


    //cout<<"features_[0] - means[0] "<<double(features_[0] - means[0])<<endl;

    cout<<"velx "<<velx<<endl;
    cout<<"vely "<<vely<<endl;
    cout<<"velz "<<velz<<endl;

    //cout<<"features_[0]  "<<features_[0]<<endl;
    //cout<<"mean: "<<means[0]<<"  var: "<<ewmaVariances[0]<<endl;

    handPos_[0] = velx;
    handPos_[1] = vely;
    handPos_[2] = velz;
}

void MovingAverageFilter::Run() {
    bool updated = false;

    ifstream baseline("../../../feature_extraction/feature_extract_cpp/build/baseline.txt");
    if (! baseline.is_open()) {
        cout<<"You need to first run a baseline recording for this static filter in order to generate /penn2/feature_extraction/feature_extract_cpp/build/baseline.txt"<<endl;
        return;
    }
    float mean, variance;
    while (baseline) {
        baseline>>mean;
        baseline>>variance;
        means.push_back(mean);
        variances.push_back(variance);
        cout<<mean<<endl;
        cout<<variance<<endl;
    }
    ewmaVariances.resize(variances.size());
    updateEwmaVariances();

    float q5=0;
    float q95=0;
    thread gui(&MovingAverageFilter::runGUI, this, ref(gamma_), ref(scale_), ref(updated_));

    ewmaValues.resize(3, 0);

  for (;;) {
      if (updated_) {
          updated_ = false;
          updateEwmaVariances();
          fwrite(&featureTimestamp_, sizeof(size_t),1 , eFile);
          fwrite(&gamma_, sizeof(float),1 , eFile);
          fwrite(&scale_, sizeof(float),1 , eFile);

          fflush(eFile);
          //q5_ = q5;
          //q95_ = q95;
      }

      SendHandPosGetState(handPos_);
      Predict();
      Update();
    }
}

/*! \brief Compute the EWMA variances
 *
 *  The EWMA variance of each channel is computed:
 *  \f[
 *      \sigma_z^2 = \sigma^2 (\frac{\alpha}{2-\alpha}) (1-(1-\alpha)^{2t})
 *  \f]
 */
void MovingAverageFilter::updateEwmaVariances() {
    const float term1 = gamma_/(2.0-gamma_);
    const float term2 = 1.0 - pow(1.0-gamma_, 1000000);
    for (unsigned i=0; i<ewmaVariances.size(); i++) {
        ewmaVariances[i] = variances[i] * term1 * term2 ;
    }
}

/*! Show the GUI window to update the value of the EWMA coefficient and
 *     the scale factor
 *
 *   \param alpha the EWMA coefficient
 *   \param scale the scale factor
 *   \param updated whether any of the values has been updated
 *
*/
void MovingAverageFilter::runGUI(float& alpha, float& scale, bool& updated) {

    using namespace boost;
    using namespace boost::accumulators;

    char buffer[20];
    char bufferScale[20];
    snprintf(buffer, 20, "%f", alpha);
    snprintf(bufferScale, 20, "%f", scale);
    Fl_Window* w = new Fl_Window(0,0,330,190, "Static Filter");
    Fl_Button ok(110,130, 100, 35, "Update");
    Fl_Input input(110, 40, 200, 25, "Kalman Gain:");
    Fl_Input inputScale(110, 80, 200, 25, "Alpha:");
    input.value(buffer);
    inputScale.value(bufferScale);
    w->end();
    w->show();

    // read the raw baseline data and run
    // compute the 5% and 95% percentiles

    vector<float> values;

    // safty zone
    // TODO: replace this with a parameter in the config file
    /*ifstream baseline("/home/user/code/penn2/penn/penn2/feature_extraction/feature_extract_cpp/build/");
    float value = 0.0;
    while (baseline) {
        baseline>>value;
        values.push_back(value);
    }*/


    while (true) {
        Fl::wait();
        Fl_Widget *o;
        while (o = Fl::readqueue()) {
            if (o == &ok) {
                strcpy(buffer, input.value());
                alpha = atof(buffer);
                strcpy(bufferScale, inputScale.value());
                scale = atof(bufferScale);

                /*int c = values.size()-1;
                accumulator_t_right accRight( boost::accumulators::tag::tail<boost::accumulators::right>::cache_size = c );
                accumulator_t_left accLeft( boost::accumulators::tag::tail<boost::accumulators::left>::cache_size = c );

                // safty zone
                /*float svalue = values[0];
                //vector<float> svalues;
                for (unsigned i=1; i < values.size(); i++) {
                    svalue = (1.0 - alpha) * svalue + alpha * double(values[i] - mean);
                    //svalues.push_back(svalue);
                    accRight(svalue);
                    accLeft(svalue);
                }

                q95_ = quantile(accRight, quantile_probability = 0.95 );
                q5_ = quantile(accLeft, quantile_probability = 0.05 );


                //q95 = lq95;
                //q5 = lq5;

                cout<<"q95: "<<q95_<<endl;
                cout<<"q5: "<<q5_<<endl;
                */

                updated = true;
            }
        }
    }
}
