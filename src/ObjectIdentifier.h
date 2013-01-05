#ifndef _OBJECT_IDENTIFIER_H_
#define _OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

static int globalID = 1;

class ObjectIdentifier {

public:
    ObjectIdentifier(Blob b);

    ~ObjectIdentifier();

    void incrementFrameCount();

    int getLastSeenNFramesAgo();

    // Returns true if blob was accepted, false if not
    virtual bool addBlob(Blob b);

    void printPoints();

    double distanceTravelled();
    double errFromLine(Blob b);
    double distanceFromLastBlob(Blob b);

    Blob getLastBlob();

    int getNumBlobs();

    int getId();

    int lifetime();

    int getFirstFrame();

    double getXMovement();
    double getYMovement();

    // Returned in pixels per frame
    double getSpeed();

    virtual bool inRange(Blob b);
    virtual double distFromExpectedPath(Blob b);
    static bool inStartingZone(Blob b);
    bool inEndZone();
    std::vector<Blob> * getBlobs();

    double errXY(double x, double y);
    double errTX(int frameNum, double x);
    double errTY(int frameNum, double y);

    double distFromExpectedY(double x, double y);
    double distFromExpectedY(double y, int frameNum);
    double distFromExpectedX(double y, int frameNum);

    double distToPredictedXY(double x, double y);
    double distToPredictedTX(int frameNum, double x);
    double distToPredictedTY(int frameNum, double y);

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

    double distanceBetweenBlobs(Blob b1, Blob b2);

    double expectedY(double x);

    double distance(double x1, double x2, double y1, double y2);

    std::pair<double,double> xyLeastSqrRegression(std::vector<Blob> &blobs, int numPointsToUse);
    std::pair<double,double> txLeastSqrRegression(std::vector<Blob> &blobs, int numPointsToUse);
    std::pair<double,double> tyLeastSqrRegression(std::vector<Blob> &blobs, int numPointsToUse);

    int lastSeen;
    int frameCount;
    int lastBlobFrameNum;
    Blob lastBlob;

    Blob closestBlob;
    double closestDistToOrigin;

    Blob furthestBlob;
    double furthestDistToOrigin;

    std::vector<Blob> blobs;
    int numBlobs;

    // Experimental
    cv::KalmanFilter& xyFilter;//(4, 2, 0);
    cv::KalmanFilter& txFilter;//(4, 2, 0);
    cv::KalmanFilter& tyFilter;//(4, 2, 0);
    cv::Mat_<float>& measurement;//(2,1);
};

#endif

