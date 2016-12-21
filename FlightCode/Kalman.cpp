// Implementation of Kalman filter in C++
// Input: Data stream
// Output: Kalman-filtered estimate of what the data should be
#include <stdio.h>
#include <stdlib.h>

class KalmanFilter {
public:
    // Initializer
    KalmanFilter(double est, double eEst, double eMeas);

    // Func
    double data(double rawData);
private:
    double estimate;
    double errEst;
    double errMeas;
};

// Definitions
KalmanFilter::KalmanFilter(double est, double eEst, double eMeas) {
    estimate = est;
    errEst = eEst;
    errMeas = eMeas;
}

double KalmanFilter::data(double rawData) {
    double kGain = errEst / (errEst + errMeas);
    estimate = estimate + kGain * (rawData - estimate);
    errEst = (1 - kGain) * errEst;

    return estimate;
}


// Test thing
// int main() {
//     KalmanFilter filter = KalmanFilter(65, 6, 6);
//     double myData [] = {75.0, 71.0, 70.0, 74.0, 68.0, 71.0, 70.0, 69.0, 69.0, 74.0, 75.0, 75.0, 71.0, 70.0, 74.0, 68.0, 71.0, 70.0, 69.0, 69.0, 74.0, 75.0,75.0, 71.0, 70.0, 74.0, 68.0, 71.0, 70.0, 69.0, 69.0, 74.0, 75.0,75.0, 71.0, 70.0, 74.0, 68.0, 71.0, 70.0, 69.0, 69.0, 74.0, 75.0};
//
//     for (int i = 0; i < 43; i++) {
//         double fdata = filter.data(myData[i]);
//         printf("%f\n", fdata);
//     }
//
//     return 0;
// }
