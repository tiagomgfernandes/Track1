#ifndef PV_FILTER_H
#define PV_FILTER_H

#include <vector>
#include <utility>
#include <ros/ros.h>

namespace cv {
    class KalmanFilter;
}

class PVFilter {

public:
    PVFilter(const double aObservationNoise, const double aVelocityNoise);
    void init(const std::vector<double>& positions, const std::vector<double>& velocities,
            const ros::Time time, const ros::Duration aDT);
    void measure(const std::vector<double>& positions, const ros::Time& time);
    void predict(std::vector<double>& positions, std::vector<double>& velocities, const ros::Time& time);
    const ros::Time& getLastUpdate() const {
        return updateTime;
    }
    unsigned int getId() const {
        return id;
    }
    unsigned int getPixels() const {
        return pixels;
    }
    void setPixels(const unsigned int aPixels){
        this->pixels = aPixels;
    }
private:
    std::auto_ptr<cv::KalmanFilter> filter;
    ros::Time updateTime;
    ros::Time predictTime;
    ros::Duration dt;
    double mVelocityNoise;
    double mObservationNoise;
    unsigned int id;
    unsigned int pixels;

    void setTransitionMatrix();
};
#endif
