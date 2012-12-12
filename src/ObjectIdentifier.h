#ifndef _OBJECT_IDENTIFIER_H_
#define _OBJECT_IDENTIFIER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"

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

static int globalID = 0;

class ObjectIdentifier {

public:
    ObjectIdentifier(CvBlob b);

    ~ObjectIdentifier();

    void incrementFrameCount();

    int getLastSeenNFramesAgo();

    // Returns true if blob was accepted, false if not
    virtual bool addBlob(CvBlob b);

    void printPoints();

    double distanceTravelled();
    double errFromLine(CvBlob b);
    double distanceFromLastBlob(CvBlob b);

    CvBlob getLastBlob();

    int getNumBlobs();

    int getId();

    int lifetime();

    int getFirstFrame();

    // Returned in pixels per frame
    double getSpeed();

    virtual bool inRange(CvBlob b);
    virtual double distFromExpectedPath(CvBlob b);
    static bool inStartingZone(CvBlob b);
    bool inEndZone();
    vector<CvBlob> * getBlobs();

private:
    int id;

    double distanceBetweenBlobs(CvBlob b1, CvBlob b2);

    double expectedY(double x);

    double distance(double x1, double x2, double y1, double y2);

    pair<double,double> leastSqrRegression(vector<CvBlob> &blobs, int numPointsToUse);

    int lastSeen;
    int frameCount;
    int lastBlobFrameNum;
    CvBlob lastBlob;

    CvBlob closestBlob;
    double closestDistToOrigin;

    CvBlob furthestBlob;
    double furthestDistToOrigin;

    vector<CvBlob> blobs;
};

#endif

