// Implementation of the Kalman filter in cpp
// Useful for reducing error in sensor output
// The first reading of a sensor shall be used as the initial guess


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
