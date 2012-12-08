#ifndef _CAR_COUNTER_H_
#define _CAR_COUNTER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "RoadObject.h"
#include "CarCounter.h"

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

// TODO: args in order, m_ as well
class CarCounter {
public:
    CarCounter(double distThresh, Rect* bounds, bool use_slope_prediction, double expPathSlope);

    ~CarCounter();

    CvBlobs getBlobs();


    void setBoundaries(Rect &bounds);

    int getMaxFrameTimeout();

    double getAvgSpeed(int numFrames);

    int updateStats(CvBlobs& blobs);



private:
    double distanceThreshold;
    double expectedPathSlope;
    int carCount;
    int bikeCount;
    int streetcarCount;
    bool useSlopeOfPathFilter;

    Rect * boundaries;

    list<RoadObject> objects;

    int rosCreated;

    const static int MIN_NUM_POINTS = 20;
    const static int MIN_FRAME_TIMEOUT = 15;
    const static int MAX_FRAME_TIMEOUT = 30 * 60 * 2; // 2 minutes at 30 fps

    const static int NUM_POINTS_FOR_LINEAR_REGRESSION = 5;
    const static int NUM_POINTS_FOR_OVERLAP_CHECK = 2;
    const static int NUM_POINTS_TO_AVG = 5;
    const static double OVERLAP_PCT_THRESHOLD = 1; // 100%
    const static double NUM_PIXELS_LINEAR_REGRESSION_THRESHOLD = 5;
    const static double BOUNDS_THRESHOLD_PCT = 0.1;
    const static int VEHICLE_SIZE_THRESHOLD = 700;
    const static double SLOPE_FILTER_THRESHOLD = 0.1;
};

#endif
