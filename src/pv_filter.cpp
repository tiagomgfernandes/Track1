#include "pv_filter.h"
#include <iostream>
#include <opencv2/video/tracking.hpp>

namespace {
using namespace std;
using namespace cv;
static unsigned int uniqueId = 0;
}

PVFilter::PVFilter(const double aObservationNoise, const double aVelocityNoise) :
                filter(new KalmanFilter(6, 3, 0)),
                mVelocityNoise(aVelocityNoise),
                mObservationNoise(aObservationNoise),
                id(uniqueId++) {
}

void PVFilter::setTransitionMatrix(){
    filter->transitionMatrix = *(Mat_<float>(6, 6) << 1, 0, 0, dt.toSec(), 0, 0,
                                                      0, 1, 0, 0, dt.toSec(), 0,
                                                      0, 0, 1, 0, 0, dt.toSec(),
                                                      0, 0, 0, 1, 0, 0,
                                                      0, 0, 0, 0, 1, 0,
                                                      0, 0, 0, 0, 0, 1);
}

void PVFilter::init(const vector<double>& positions,
        const vector<double>& velocities,
        ros::Time time,
        ros::Duration aDT) {

    updateTime = time;
    predictTime = time;

    dt = aDT;
    setTransitionMatrix();

    setIdentity(filter->measurementMatrix);
    setIdentity(filter->processNoiseCov, Scalar::all(mVelocityNoise));
    setIdentity(filter->measurementNoiseCov, Scalar::all(mObservationNoise));
    setIdentity(filter->errorCovPost, Scalar::all(1));

    filter->statePost.at<float>(0) = positions[0];
    filter->statePost.at<float>(1) = positions[1];
    filter->statePost.at<float>(2) = positions[2];
    filter->statePost.at<float>(3) = velocities[0];
    filter->statePost.at<float>(4) = velocities[1];
    filter->statePost.at<float>(5) = velocities[2];
}

void PVFilter::measure(const vector<double>& positions, const ros::Time& time) {
    Mat measurement = Mat::zeros(3, 1, CV_32F);

    measurement.at<float>(0) = positions[0];
    measurement.at<float>(1) = positions[1];
    measurement.at<float>(2) = positions[2];
    filter->correct(measurement);
    // TODO: Does the measurement need to compensate for the time passed?
    updateTime = time;
}

void PVFilter::predict(vector<double>& positions, vector<double>& velocities, const ros::Time& time) {
    ros::Duration delta = time - predictTime;

    // TODO: We should not predict here
    if(delta < ros::Duration(0.0)){
        ROS_INFO("Negative delta in prediction");
        delta = ros::Duration(0.0);
    }
    ROS_DEBUG("Predicting results between %f and %f with dt %f", predictTime.toSec(), time.toSec(), dt.toSec());

    positions.resize(3);
    velocities.resize(3);

    for(ros::Duration t(0.0); t <= delta; t += dt){
        const Mat& prediction = filter->predict();
        positions[0] = prediction.at<float>(0);
        positions[1] = prediction.at<float>(1);
        positions[2] = prediction.at<float>(2);
        velocities[0] = prediction.at<float>(3);
        velocities[1] = prediction.at<float>(4);
        velocities[2] = prediction.at<float>(5);
    }
    predictTime =  time;
}


