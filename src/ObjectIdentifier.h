#ifndef _OBJECT_IDENTIFIER_H_
#define _OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

static int globalID = 1;

class ObjectIdentifier {

public:
    ObjectIdentifier(Blob* b);

    virtual ~ObjectIdentifier();

    // Returns fitness score. Higher is better.
    virtual int getFit(Blob& b);

    void updateTime(long currentTime);

    long lastSeen();

    virtual long getTimeout() = 0;

    virtual bool addBlob(Blob& b);

    void printPoints();

    double getXYSlope();

    double getDistanceTravelled();

    int getNumBlobs();

    int getId();

    long getLifetime();

    enum ObjectType {
        UNKNOWN,
        VEHICLE,
        STREET_CAR,
        PERSON,
        BICYCLE,
        NUM_CLASSES
    };

    virtual ObjectType getType() = 0;

    std::vector<Blob*>& getBlobs();

protected:
    double errFromLine(Blob& b);
    double distanceFromLastBlob(Blob& b);

    Blob& getLastBlob();

    int getFirstTime();

    double getXMovement();
    double getYMovement();

    // Returned in pixels per millisecond
    double getSpeed();

    bool inEndZone();

    double errXY(double x, double y);
    double errTX(long time, double x);
    double errTY(long time, double y);

    double distFromExpectedY(double x, double y);
    double distFromExpectedY(double y, long time);
    double distFromExpectedX(double y, long time);

    double distToPredictedXY(double x, double y);
    double distToPredictedTX(long time, double x);
    double distToPredictedTY(long time, double y);

    double getAverageSize();

    double rValues()
    {
        return xyR + tyR + txR;
    }

    // TODO: make private
    double xyR;
    double tyR;
    double txR;

private:
    int id;

    double distanceBetweenBlobs(Blob& b1, Blob& b2);

    double expectedY(double x);

    double distance(double x1, double x2, double y1, double y2);

    std::pair<double,double> xyLeastSqrRegression(std::vector<Blob*> &blobs, int numPointsToUse);
    std::pair<double,double> txLeastSqrRegression(std::vector<Blob*> &blobs, int numPointsToUse);
    std::pair<double,double> tyLeastSqrRegression(std::vector<Blob*> &blobs, int numPointsToUse);

    long currentTime;
    long startTime;
    int frameCount;
    Blob* lastBlob;

    Blob* closestBlob;
    double closestDistToOrigin;

    Blob* furthestBlob;
    double furthestDistToOrigin;

    std::vector<Blob*> blobs;
    int numBlobs;

    // Experimental
    cv::KalmanFilter& xyFilter;//(4, 2, 0);
    cv::KalmanFilter& txFilter;//(4, 2, 0);
    cv::KalmanFilter& tyFilter;//(4, 2, 0);
    //cv::Mat_<float>& measurement;//(2,1);

    ObjectType type;
};

#endif

