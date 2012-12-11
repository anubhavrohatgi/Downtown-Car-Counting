#ifndef _ROAD_OBJECT_H_
#define _ROAD_OBJECT_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "RoadObject.h"

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

class RoadObject {

public:
    RoadObject(int inID, CvBlob b);

    ~RoadObject();

    Rect getBounds();

    double speedPixelsPerFrame();

    double speedPixelsPerFrame(int lastNFrames);

    double size();

    void incrementFrameCount();

    int getLastSeenNFramesAgo();

    void addTrack(CvBlob b);

    void printPoints();

    double slopeOfPath();

    // NOTE: Not exactly accurate ... measures last blobs, regardless when exactly they were recorded.
    // FIXME
    double distanceTravelledLastNFrames(int numFrames);

    double distanceTravelled();

    double errFromLine(int numPoints, CvBlob b);

    double distanceFromLastPoint(int numPoints, CvBlob p);

    CvBlob getLastBlob();

    int getNumPoints();

    int getId();

    vector<CvBlob> * getPoints();

private:
    double distanceBetweenPoints(CvBlob b1, CvBlob b2);

    double distance(double x1, double x2, double y1, double y2);

    int lastSeen;
    int frameCount;
    int lastBlobFrameNum;
    CvBlob lastPoint;
    vector<CvBlob> points;
    int id;
    double minx, maxx, miny, maxy;
};

#endif

