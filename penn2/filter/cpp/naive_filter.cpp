/*! \file naive_filter.cpp
      \brief Implements the static filter
*/

#include "naive_filter.h"
#include <iostream>
#include <thread>
#include <fstream>

#include <Fl/Fl.H>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <FL/Fl_Output.H>

using namespace std;

NaiveFilter::NaiveFilter(float featureRate){
  featureRate_ = featureRate;
  alpha_ = 0.9;
  scale_ = 5.0;
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
    double velx = double(features_[0] - meanX) * scaleX;
    double vely = 0; //double(features_[1] - means[1]) / (3.0 * sqrt(ewmaVariances[1]));
    double velz = 0; //double(features_[2] - means[2]) / (3.0 * sqrt(ewmaVariances[2]));

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

void NaiveFilter::Run() {
    bool updated = false;
    thread gui(&NaiveFilter::runGUI, this, ref(alpha_), ref(scale_), ref(updated));

    ifstream baseline("/home/user/code/penn2/penn/penn2/feature_extraction/feature_extract_cpp/build/baseline.txt");
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

    ewmaValues.resize(3, 0);

  for (;;) {
      if (updated) {
          updated = false;
          updateEwmaVariances();
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
void NaiveFilter::updateEwmaVariances() {
    const float term1 = alpha_/(2.0-alpha_);
    const float term2 = 1.0 - pow(1.0-alpha_, 1000000);
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
void NaiveFilter::runGUI(float& alpha, float& scale, bool& updated) {
    char buffer[20];
    char bufferScale[20];
    snprintf(buffer, 20, "%f", alpha);
    snprintf(bufferScale, 20, "%f", scale);
    Fl_Window* w = new Fl_Window(0,0,330,190, "Static Filter");
    Fl_Button ok(110,130, 100, 35, "Update");
    Fl_Input input(60, 40, 250, 25, "Alpha:");
    Fl_Input inputScale(60, 80, 250, 25, "Scale:");
    input.value(buffer);
    inputScale.value(bufferScale);
    w->end();
    w->show();

    while (true) {
        Fl::wait();
        Fl_Widget *o;
        while (o = Fl::readqueue()) {
            if (o == &ok) {
                strcpy(buffer, input.value());
                alpha = atof(buffer);
                strcpy(bufferScale, inputScale.value());
                scale = atof(bufferScale);
                updated = true;
            }
        }
    }
}
