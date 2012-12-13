#ifndef _CAR_COUNTER_H_
#define _CAR_COUNTER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "Blob.h"
#include "ObjectIdentifier.h"
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
    CarCounter();

    ~CarCounter();

    int getCarCount() {
        return carCount;
    }

    double getAvgSpeed(int numFrames);

    int updateStats(std::vector<Blob>& blobs);
    int updateStats(std::vector<Blob>& blobs, int frameNum);


private:
    double distanceThreshold;
    double expectedPathSlope;
    int carCount;
    int bikeCount;
    int streetcarCount;
    int xBoundary;
    bool useSlopeOfPathFilter;

    Rect * boundaries;

    list<ObjectIdentifier> objects;
    list<Blob> unidentifiedBlobs;

    vector<Blob> allBlobs;

    int rosCreated;

    int frameNumber;

    const static int MIN_NUM_POINTS = 7;
    const static int MIN_FRAME_TIMEOUT = 5 * 10; // 10 seconds at 5 fps
    const static int MAX_FRAME_TIMEOUT = 5 * 60 * 2; // 2 minutes at 5 fps

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
