#ifndef _OBJECT_IDENTIFIER_H_
#define _OBJECT_IDENTIFIER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "Blob.h"
#include "Blob.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/flann/flann_base.hpp"

/* time example */
#include <stdio.h>
#include <time.h>

#include <stdio.h>

using namespace cv;

using namespace std;
using namespace cvb;

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

    // Returned in pixels per frame
    double getSpeed();

    virtual bool inRange(Blob b);
    virtual double distFromExpectedPath(Blob b);
    static bool inStartingZone(Blob b);
    bool inEndZone();
    vector<Blob> * getBlobs();

    double errXY(double x, double y);
    double errTX(int frameNum, double x);
    double errTY(int frameNum, double y);

    double distFromExpectedY(double x, double y);
    double distFromExpectedY(double y, int frameNum);
    double distFromExpectedX(double y, int frameNum);

    bool continuesTrend(Blob b);


private:
    int id;

    double distanceBetweenBlobs(Blob b1, Blob b2);

    double expectedY(double x);

    double distance(double x1, double x2, double y1, double y2);

    pair<double,double> xyLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse);
    pair<double,double> tyLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse);
    pair<double,double> txLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse);

    int lastSeen;
    int frameCount;
    int lastBlobFrameNum;
    Blob lastBlob;

    Blob closestBlob;
    double closestDistToOrigin;

    Blob furthestBlob;
    double furthestDistToOrigin;

    vector<Blob> blobs;
};

#endif

