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

// FIXME
#include "LinearRegression.cpp"

pair<double,double> leastSqrRegression(vector<CvBlob> &points, int numPointsToUse);

class RoadObject {

public:
    RoadObject(int inID, CvBlob b) :
        lastSeen(0),
        lastPoint(b),
        id(inID),
        minx(b.minx),
        maxx(b.maxx),
        miny(b.miny),
        maxy(b.maxy),
        frameCount(0),
        lastBlobFrameNum(0)
    {
        //printf("Creating %d ... (%02f,%02f,%02f,%02f)\n", id, minx, maxx, miny, maxy);
        points.push_back(b);
    }

    ~RoadObject()
    {
        printf("~%d (#pts %d): (%.2f, %.2f, %.2f, %.2f) size %.2f\n", id, points.size(), minx, maxx, miny, maxy, size());
    }

    Rect getBounds()
    {
        return Rect(minx, miny, maxx - minx, maxy - miny);
    }

    double speedPixelsPerFrame()
    {
        return (distanceTravelled() / ((lastBlobFrameNum > 0) ? lastBlobFrameNum : 1));
    }

    double speedPixelsPerFrame(int lastNFrames)
    {
        if (lastNFrames < 2) return 0;
        return (distanceTravelledLastNFrames(lastNFrames) / lastNFrames);
    }

    double size()
    {
        double areaSum = 0;
        for (int i = 0; i < points.size(); i++) {
            areaSum += points.at(i).area;
        }
        return areaSum / points.size();
    }

    void incrementFrameCount()
    {
        frameCount++;
        lastSeen++;
    }

    int getLastSeenNFramesAgo()
    {
        return lastSeen;
    }

    void addBlob(CvBlob b)
    {
        lastPoint = b;
        points.push_back(b);
        lastSeen = 0;
        lastBlobFrameNum = frameCount;

        minx = (b.minx < minx) ? b.minx : minx;
        maxx = (b.maxx > maxx) ? b.maxx : maxx;
        miny = (b.miny < miny) ? b.miny : miny;
        maxy = (b.maxy > maxy) ? b.maxy : maxy;
    }

    void printPoints()
    {
        for (int i = 0; i < points.size(); i++) {
            printf("%d,%d\n", points.at(i).centroid.x, points.at(i).centroid.y);
        }
    }

    double pctRecentOverlap(int numPoints, CvBlob r2)
    {
        int overlaps = 0;
        int numPointToAvg = (points.size() < numPoints) ? points.size() : numPoints;

        for (int i = 0; i < numPointToAvg; i++) {
            CvBlob r1 = points.at(points.size() - 1 - i);

            //printf("R1 %d %d %d %d\n", r1.minx, r1.maxx, r1.miny, r1.maxy);
            //printf("R2 %d %d %d %d\n", r2.minx, r2.maxx, r2.miny, r2.maxy);

            // http://www.leetcode.com/2011/05/determine-if-two-rectangles-overlap.html

            bool noOverlap = ( r1.maxy < r2.miny || r1.miny > r2.maxy || r1.maxx < r2.minx || r1.minx > r2.maxx );

                    //( r1.maxx < r2.maxy || r1.maxy > r2.miny || r1.maxx < r2.minx || r1.minx > r2.maxx );

            //printf("%d %d %d %d\n", r1.maxy < r2.miny, r1.miny > r2.maxy, r1.maxx < r2.minx, r1.minx > r2.maxx);

#if 0
            if (rect1.topLeft.x >= rect2.bottomRight.x
                || rect1.bottomRight.x <= rect2.topLeft.x
                || rect1.topLeft.y <= rect2.bottomRight.y
                || rect1.bottomRight.y >= rect2.topLeft.y)
              return false;

            ALT
            if (rect1.topLeft.x >= rect2.bottomRight.x
            || rect1.bottomRight.x <= rect2.topLeft.x
            || rect1.topLeft.y >= rect2.bottomRight.y
            || rect1.bottomRight.y <= rect2.topLeft.y)
#endif

            if (!noOverlap) {
                overlaps++;
            }
        }
        return (numPointToAvg > 0) ? (overlaps / numPointToAvg) : 0;
    }

    double slopeOfPath()
    {
        return leastSqrRegression(points, points.size()).first;
    }

    // NOTE: Not exactly accurate ... measures last blobs, regardless when exactly they were recorded.
    // FIXME
    double distanceTravelledLastNFrames(int numFrames)
    {
        if (getLastSeenNFramesAgo() >= numFrames) return 0;

        numFrames = (numFrames <= points.size()) ? numFrames : points.size();
        double dminx = 99999; // MAX_INT
        double dmaxx = 0;
        double dminy = 99999; // MAX_INT
        double dmaxy = 0;
        for (int i = 0; i < numFrames; i++) {
            dminx = (points.at(i).centroid.x < dminx) ? points.at(i).centroid.x : dminx;
            dmaxx = (points.at(i).centroid.x > dmaxx) ? points.at(i).centroid.x : dminx;
            dminy = (points.at(i).centroid.x < dminy) ? points.at(i).centroid.y : dminy;
            dmaxy = (points.at(i).centroid.x > dmaxy) ? points.at(i).centroid.y : dmaxy;
        }
        return distance(dminx, dmaxx, dminy, dmaxy);
    }

    double distanceTravelled()
    {
        return distance(minx, maxx, miny, maxy);
    }

    double errFromLine(int numPoints, CvBlob b)
    {
        if (numPoints == 1 || points.size() == 1) {
            return 999999; // MAX_INT
        }

        pair<double,double> line = leastSqrRegression(points, numPoints);
        double slope = line.first;
        double y_int = line.second;
        double y_exp = slope * b.centroid.x + y_int;
        return (abs(b.centroid.y - y_exp));
    }

    double distanceFromLastPoint(int numPoints, CvBlob p)
    {
        int numPointToAvg = (points.size() < numPoints) ? points.size() : numPoints;
        double distanceSum = 0;
        for (int i = 0; i < numPointToAvg; i++) {
            distanceSum += distanceBetweenPoints(p, points.at(points.size() - 1 - i));
        }
        return (distanceSum / numPointToAvg);
    }

    CvBlob getLastBlob()
    {
        return lastPoint;
    }

    int getNumPoints()
    {
        return points.size();
    }

    int getId()
    {
        return id;
    }

    vector<CvBlob> * getPoints()
    {
        return &points;
    }

private:
    double distanceBetweenPoints(CvBlob b1, CvBlob b2)
    {
        double dist = 0;
        if (points.size() != 0) {
            dist = distance(b1.centroid.x, b2.centroid.x, b1.centroid.y, b2.centroid.y);
        }
        return dist;
    }

    double distance(double x1, double x2, double y1, double y2)
    {
        return sqrt(abs((x2 - x1) * (x2 - x1)) + abs((y2 - y1) * (y2 - y1)));
    }

    int lastSeen;
    int frameCount;
    int lastBlobFrameNum;
    CvBlob lastPoint;
    vector<CvBlob> points;
    int id;
    double minx, maxx, miny, maxy;
};
// TODO: args in order, m_ as well
class CarCounter {
public:
    CarCounter(double distThresh=50, Rect* bounds=NULL, bool use_slope_prediction=false, double expPathSlope=0.2) :
        distanceThreshold(distThresh),
        carCount(0),
        bikeCount(0),
        streetcarCount(0),
        rosCreated(0),
        boundaries(bounds),
        useSlopeOfPathFilter(use_slope_prediction),
        expectedPathSlope(expPathSlope)
    {

    }

    ~CarCounter()
    {

    }

    CvBlobs getBlobs()
    {
        CvBlobs blobs;
        for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {
            RoadObject * ro = &*obj;
            pair<const unsigned int, CvBlob*> blob(ro->getLastBlob().label, &ro->getLastBlob());
            blobs.insert(blob);
        }
    }


    void setBoundaries(Rect &bounds) {
        boundaries = &bounds;
    }

    int getMaxFrameTimeout() {
        return MAX_FRAME_TIMEOUT;
    }

    double getAvgSpeed(int numFrames)
    {
        if (numFrames < 2) return 0;

        double speedSum = 0;
        for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {
            RoadObject * ro = &*obj;
            speedSum += ro->speedPixelsPerFrame(numFrames);
        }
        return (speedSum / numFrames);
    }

    int updateStats(CvBlobs& blobs) {
        int newROs = 0;
        CvBlobs::iterator it = blobs.begin();

        if (objects.size() == 0 && blobs.size() != 0) {
            //printf ("NO ROs: CREATE NEW RO\n");
            // TODO: better memory mgmt - this is copied and deleted

            // TODO: Remove code duplication
            CvBlob * blob = blobs.begin()->second;
            if (boundaries) {
                // Only create new Road Objects that are near a boundary ... we don't have cars magically appear
                // in the middle of the road ...
                double pixelLimit = boundaries->width * BOUNDS_THRESHOLD_PCT;
                double deltaX1 = abs(blob->centroid.x - boundaries->x);
                double deltaX2 = abs((blob->centroid.x) - (boundaries->x + boundaries->width));
                if (deltaX1 > pixelLimit && deltaX2 > pixelLimit) return 0;
            }

            RoadObject obj(rosCreated++, *blob);
            objects.push_back(obj);
            it++;
            //newROs++;
        }

        for (; it != blobs.end(); ++it) {

            CvBlob blob = *(it->second);
#if 1
            RoadObject * closestFit = NULL;
            RoadObject * bestOverlap = NULL;

            double minDistance = 999999; //MAX_INT
            double maxOverlap = 0;

            for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

                // Road Object we're testing
                RoadObject * ro = &*obj;

                // Track RO that is the closest match
                double distanceFromLastPt = ro->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob);
                if (distanceFromLastPt < minDistance) {
                    minDistance = distanceFromLastPt;
                    closestFit = ro;
                }

                double overlap = ro->pctRecentOverlap(NUM_POINTS_FOR_OVERLAP_CHECK, blob);
                if (overlap > maxOverlap) {
                    maxOverlap = overlap;
                    bestOverlap = ro;
                }

                //printf("BLOB %d CHECK RO %d DIST %f MIN_DST %f OVERLAP_PCT %f\n", blob.label, ro->getId(), distanceFromLastPt, minDistance, overlap);
            }

            // Look at metrics calculated and find the best RO to add the blob to, or create a new one
            if (closestFit && minDistance < distanceThreshold) {
                //printf("DISTANCE - ADD TO ID %d dist %f\n", closestFit->getId(), minDistance);
                closestFit->addBlob(blob);
            } else if (bestOverlap && maxOverlap >= OVERLAP_PCT_THRESHOLD) {
                //printf("OVERLAP - ADD TO ID %d overlap %f\n", bestOverlap->getId(), maxOverlap);
                bestOverlap->addBlob(blob);
            } else {
                double fitToOverlapRO = 999999;
                double fitToClosestRO = 999999;

                // Check linear regression as long as we're within 2X the distance threshold
                if (bestOverlap && (bestOverlap->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob) <= 2 * distanceThreshold)) {
                    fitToOverlapRO = bestOverlap->errFromLine(NUM_POINTS_FOR_LINEAR_REGRESSION, blob);
                }

                if (closestFit && (closestFit->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob) <= 2 * distanceThreshold)) {
                    fitToClosestRO = closestFit->errFromLine(NUM_POINTS_FOR_LINEAR_REGRESSION, blob);
                }

                if (fitToOverlapRO < NUM_PIXELS_LINEAR_REGRESSION_THRESHOLD || fitToClosestRO < NUM_PIXELS_LINEAR_REGRESSION_THRESHOLD) {
                    if (fitToOverlapRO < fitToClosestRO) {
                        //printf("LINEAR_REGRESSION (BEST_OVERLAP) ADD TO ID %d dist_to_line %f\n", bestOverlap->getId(), fitToOverlapRO);
                        bestOverlap->addBlob(blob);
                    } else {
                        //printf("LINEAR_REGRESSION (CLOSEST_FIT) ADD TO ID %d dist_to_line %f\n", closestFit->getId(), fitToClosestRO);
                        closestFit->addBlob(blob);
                    }
                } else {
                    RoadObject ro(rosCreated++, blob);
                    printf("CREATE RO %d blob size %d at (%f,%f)\n", ro.getId(), it->second->area, blob.centroid.x, blob.centroid.y);
                    newROs++;
                    objects.push_back(ro);
                }

            }
#endif
        }

        // Iterate through RoadObjects and see if we can classify (or discard) them
        for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            RoadObject * ro = &*obj;
            ro->incrementFrameCount(); // Update 'age' counter
            int lastSeen = ro->getLastSeenNFramesAgo();

            if (ro->getNumPoints() >= MIN_NUM_POINTS && lastSeen > MIN_FRAME_TIMEOUT && lastSeen < MAX_FRAME_TIMEOUT) {

                // Road object has disappeared for a bit, check if we can classify it yet ...
                bool pathWithinBounds = true;
                if (boundaries) {
                    Rect objBounds = ro->getBounds();
                    // Note: I am only checking x bounds, but y could also be checked
                    // This makes sure the object has roughly travelled where we expect it to have to be a vehicle of some type
                    double pixelLimit = boundaries->width * BOUNDS_THRESHOLD_PCT;
                    double deltaX1 = abs(objBounds.x - boundaries->x);
                    double deltaX2 = abs((objBounds.x + objBounds.width) - (boundaries->x + boundaries->width));
                    pathWithinBounds = (deltaX1 < pixelLimit && deltaX2 < pixelLimit);
                    //printf("deltaX1 %.2f deltaX2 %.2f limit %.2f Within Bounds: %d\n", deltaX1, deltaX2, pixelLimit, pathWithinBounds);
                }

                bool slopeWithinRange = true;
                if (useSlopeOfPathFilter) {
                    double slope = ro->slopeOfPath();
                    if (slope < (expectedPathSlope - SLOPE_FILTER_THRESHOLD) || slope > (expectedPathSlope + SLOPE_FILTER_THRESHOLD)) {
                        slopeWithinRange = false;
                    }
                    //printf("Slope %f within rage? %d\n", slope, slopeWithinRange);

                }

                if (pathWithinBounds && slopeWithinRange) {
                    if (ro->size() > VEHICLE_SIZE_THRESHOLD) {
                        carCount++;
                    } else {
                        bikeCount++;
                    }
                    printf("Adding ... np %d dist %f size %f slope %f speed (pixels/frame) %.2f\n", ro->getNumPoints(), ro->distanceTravelled(), ro->size(), ro->slopeOfPath(), ro->speedPixelsPerFrame());
                    printf("Car Count: %d Bike Count %d\n", carCount, bikeCount);
                    newROs++;
                    obj = objects.erase(obj);
                }
            } else if (ro->getLastSeenNFramesAgo() >= MAX_FRAME_TIMEOUT) {
                // TODO: check if we can call this a questionable car
                obj = objects.erase(obj);
                newROs++;
            }
        }
        return newROs;
    }



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

#include <sys/time.h>

// Hack to measure how long certain operations take (in ms and us)
// Call profileStart() followed by profileEnd() and it'll print the diff of those call times
timeval profile;
long profile_start, profile_end;

void profileStart() {
    gettimeofday(&profile, NULL);
    profile_start = (profile.tv_sec * 1000 * 1000) + (profile.tv_usec);
}

void profileEnd() {
    gettimeofday(&profile, NULL);
    profile_end = (profile.tv_sec * 1000 * 1000) + (profile.tv_usec);
    printf("Profile: %d us %d ms\n", profile_end - profile_start,
            (profile_end - profile_start) / 1000);
}

int write_to_file(char const *fileName, char * line)
{
    printf("To File: %s", line);
    FILE *f = fopen(fileName, "a");
    if (f == NULL) return -1;
    // you might want to check for out-of-disk-space here, too
    fprintf(f, "%s", line);
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage: %s PATH_TO_RAW_AVI_VIDEO_FILE [IMAGE_MASK_JPG_OR_OTHER_FORMAT]", argv[0]);
        return 0;
    }
    // TODO: use string?
    char * video_filename = argv[1];

    bool useImgMask = false;
    if (argc > 2) {
        useImgMask = true;
    }
    //     //sept9_raw60

    char buf[80];
    time_t t;
    time(&t);
    sprintf(buf, "%s-data(%d).csv", video_filename, t);
    char * data_filename = buf;
    //filename = "/Users/j3bennet/afternoon_raw_cut.avi";

    CvCapture* capture = cvCaptureFromAVI(video_filename);

    namedWindow("Capture ", CV_WINDOW_AUTOSIZE);
    namedWindow("Background", CV_WINDOW_AUTOSIZE);
    Mat aframe, foreground, image;
    BackgroundSubtractorMOG2 mog;

    //capture = cvCaptureFromCAM(0); // capture from video device #0
    //Capturing a frame:
    //IplImage* img = 0;

    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    cv::BackgroundSubtractorMOG2 bg(0, 7, false);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;

    std::vector<std::vector<cv::Point> > contours;

    Mat eastbound_mask;
    if (useImgMask) {
        eastbound_mask = imread(argv[2],CV_LOAD_IMAGE_ANYCOLOR); // Read the file
    }
    //Mat eastbound_mask = imread("/Users/j3bennet/eastbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat eastbound_mask = imread("/Users/j3bennet/westbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat westbound_mask = imread("/Users/j3bennet/king_st_westbound_mask.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat bgImgGrey = imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
    Mat bgImg =
            imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_ANYCOLOR); // Read the file


    /* get fps, needed to set the delay */
    int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
    int totalFrame = (int) cvGetCaptureProperty(capture,
            CV_CAP_PROP_FRAME_COUNT);

    /* display video */
    //    cvNamedWindow( "video", 0 );
    //    cvNamedWindow("foreground", 0);

    IplImage *orig;
    int key;

    //Mat bg_filtered;
    //cv::add(bgImgGrey, mask, bg_filtered);

    CvBlobs blobs;
    CvTracks tracks;

#if 0
    namedWindow("image", CV_WINDOW_NORMAL);
    namedWindow("foreground mask", CV_WINDOW_NORMAL);
    namedWindow("foreground image", CV_WINDOW_NORMAL);
    namedWindow("mean background image", CV_WINDOW_NORMAL);
#endif

    BackgroundSubtractorMOG2 bg_model;//(100, 3, 0.3, 5);
    Mat img, fgmask, fgimg;

    //Mat bg_filtered;
    //cv::add(bgImg, eastbound_mask, bg_filtered);
#if 0
    //update the model
    for (int i = 0; i < 100; i++) {
        bool update_bg_model = true;
        bg_model(bg_filtered, fgmask, update_bg_model ? -1 : 0);
        fgimg = Scalar::all(0);
        bg_filtered.copyTo(fgimg, fgmask);

    }
#endif

    int frameCount = 0;

    // Eastbound Params
    //Rect east_traffic_boundaries(275, 300, (610-275), (410-300));
    //double expectedPathSlope = 0.2; // From experimentation
    //CarCounter counter(50, &east_traffic_boundaries, true, expectedPathSlope);

    // Westbound Params
    Rect westTrafficBoundaries(260, 250, (600-260), (350-250));
    double expectedPathSlope = 0.2; // From experimentation
    CarCounter counter(50, &westTrafficBoundaries, true, expectedPathSlope);

    CvSize vga;
    vga.height = 480;
    vga.width = 640;

    timeval time;

    gettimeofday(&time, NULL);
    long last_run = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    long start_time = last_run;
    int min = 999, sum = 0, max = 0;

    namedWindow("video", CV_WINDOW_NORMAL);
    namedWindow("fg-before", CV_WINDOW_NORMAL);
    namedWindow("fg-after", CV_WINDOW_NORMAL);
    namedWindow("fgmask", CV_WINDOW_NORMAL);

    int actionFrameCount = 0;

    // Don't start processing immeditely, need time to learn background and such
    bool stateProcessCounts = false;
    static int BG_LEARNING_TIME_MS = 1000;

    int bgLearningFramesReqd = (fps * 1000) / BG_LEARNING_TIME_MS;
    int sequentialLearningFrames = 0;

    while (key != 'q' && frameCount < totalFrame) {
        /* get a frame */
        orig = cvQueryFrame(capture);

        cv::Mat frame(orig, true);// = Mat(orig, true);
        frameCount++;

        // Mask Image
        Mat masked;
        try {

#if 1

#if 0
            // SET ROI
            cv::Rect roi = cv::Rect((img2.cols - img1.cols)/2,(img2.rows - img1.rows)/2,img1.cols,img1.rows);

            cv::Mat roiImg;
            roiImg = img2(roi);

            img1.copyTo(roiImg);
#endif
            if (useImgMask) {
                cv::add(frame, eastbound_mask, masked);
            } else {
                frame.copyTo(masked); // TODO: fix up these names and such
            }
            cv::imshow("video", frame);

            // Background Subtraction
            if (fgimg.empty())
                fgimg.create(img.size(), img.type());

            //update the model
            bool update_bg_model = true; //(blobs.size() > 0 && frameCount > 300) ? false : true;
            bg_model(masked, fgmask, update_bg_model ? -1 : 0);

            fgimg = Scalar::all(0);
            masked.copyTo(fgimg, fgmask);
            //filtered.copyTo(fgimg, fgimg);

            Mat bgimg;
            bg_model.getBackgroundImage(bgimg);

            //imshow("fg-before", fgimg);

            //cv::erode(fgimg,fgimg,cv::Mat()); // doesn't seem to help much - makes black zones
            //cv::dilate(fgimg,fgimg,cv::Mat()); // makes img more pixelated

#if 0
            // Doesn't seem to help much
            // Pyramid Transform
            CvSize sz;
            sz.height = 480/2;
            sz.width = 640/2;
            Mat downSampled;
            downSampled.create(sz, fgimg.type());
            pyrDown(fgimg, downSampled,downSampled.size());
            //void cvPyrDown( const CvArr* src, CvArr* dst, int filter=CV_GAUSSIAN_5x5);

            // Smoothing
            // Soft smoothing
            Mat smoothed;
            smoothed.create(fgimg.size(), fgimg.type());
            //cvtColor(fgimg, fgimg, CV_BGR2HSV);
            bilateralFilter(fgimg, smoothed, 5, 20,20);
#endif

            //               cvtColor(fgimg, fgimg, CV_BGR2HSV);
            //               cvCvtColor(img, imgHSV, CV_BGR2HSV); // convert the coming image from the camera from RGB format to HSV (Hue, Saturation, Value)
            ///               IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1); //hold the thresholded image
            //               cvSmooth(imgHSV, imgHSV, CV_GAUSSIAN, 11, 11);//smooth the image to remove the noise from the image

            cvtColor(fgimg, fgimg, CV_BGR2GRAY);
            //threshold(fgimg, fgimg, 100, 255, 3);
            //fgimg = GetThresholdedImage(fgimg);
            IplImage filtered_img = fgimg;

#if 1
            IplImage *labelImg = cvCreateImage(vga, IPL_DEPTH_LABEL, 1);
            IplImage *dstImg = cvCreateImage(vga, IPL_DEPTH_8U, 3);

            unsigned int result = cvLabel(&filtered_img, labelImg, blobs);

            cvFilterByArea(blobs, 200, 10000);
            int newROs = 0;

            //printf("%d\n", blobs.size());

            if (!stateProcessCounts) {
                if (blobs.size() == 0) {
                    sequentialLearningFrames++;
                } else {
                    sequentialLearningFrames = 0;
                }

                if (sequentialLearningFrames == bgLearningFramesReqd) {
                    stateProcessCounts = true;
                    printf("BG Learning Complete ... starting to count cars!\n");
                }
            }

            if (stateProcessCounts) {
                newROs = counter.updateStats(blobs);
                //printf("AVG SPEED %.2f\n", counter.getAvgSpeed(2));
            }

            if (totalFrame == frameCount) {
                // Video clip is done, let everything timeout ...
                printf("Starting timeout sequence ...\n");
                blobs.clear();
                for (int i = 0; i < counter.getMaxFrameTimeout(); i++) {
                    newROs = counter.updateStats(blobs);
                }
            }

            //CvLabel index = cvLargestBlob(blobs);
            //printf("%d,%d,%d,%d\n",frameCount, blobs.size(), index, index ? blobs.at(index)->area : 0);

            cvRenderBlobs(labelImg, blobs, &filtered_img, dstImg);

            for (CvBlobs::iterator it = blobs.begin(); it != blobs.end(); ++it) {

                        CvBlob blob = *(it->second);
            #if 0
                        cout << "Blob #" << it->second->label << ": Area="
                                << it->second->area << ", Centroid=("
                                << it->second->centroid.x << ", "
                                << it->second->centroid.y << ")" << endl;
                        printf("BOUNDS %d %d %d %d\n", blob.minx, blob.maxx, blob.miny, blob.maxy);
            #endif
                    }

            if (blobs.size() > 0) {
                actionFrameCount++;
            }
#endif
            cvShowImage("dstImg", dstImg);
            //imshow("fg-after", fgimg);
            cvReleaseImage(&labelImg);
            cvReleaseImage(&dstImg);
            //imshow("fg-after", filtered_img);
            //imshow("fgmask", fgmask);

            //imshow("image", frame);
            //imshow("foreground mask", fgmask);
            //imshow("foreground image", fgimg);
            if (!bgimg.empty()) {
                //imshow("mean background image", bgimg );
            }
#endif

            key = cvWaitKey(1);
             if (key == 'p' || newROs != 0) {
                 do {
                     key = cvWaitKey(1);
                 } while (key != 'p');
             }


        } catch (cv::Exception& e) {
            printf("Caught Exception: %s\n", e.what());
        }

        // Keep track of timing
        // ms timing on Mac and Windows: http://brian.pontarelli.com/2009/01/05/getting-the-current-system-time-in-milliseconds-with-c/
        gettimeofday(&time, NULL);
        long this_run = (time.tv_sec * 1000) + (time.tv_usec / 1000);
        int processingTime = this_run - last_run;
        sum += processingTime;
        min = (processingTime < min) ? processingTime : min;
        max = (processingTime > max) ? processingTime : max;
        last_run = this_run;

        // Needed to display a video image.  performance: fps from 27 to 22
        //printf("Frame %d/%d (min,avg,max) (%d,%d,%d) ms fps=%d\n", frameCount, totalFrame, min, sum/frameCount, max, 1000*frameCount/sum);
    }

#if 0
    //Distance::ElementType d;
    Mat A(500, 2, DataType<float>::type);

    cvflann::Matrix<Distance> m;

    cvflann::flann_distance_t

    //Mat points<Distance::ElementType>;
    cvflann::hierarchicalClustering
#endif
    printf("Frame %d (min,avg,max) (%d,%d,%d) ms fps=%d\n", frameCount, min, sum/frameCount, max, 1000*frameCount/sum);

    /* free memory */
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
}

pair<double,double> leastSqrRegression(vector<CvBlob> &points, int numPointsToUse)
{
   double SUMx = 0;     //sum of x values
   double SUMy = 0;     //sum of y values
   double SUMxy = 0;    //sum of x * y
   double SUMxx = 0;    //sum of x^2
   double SUMres = 0;   //sum of squared residue
   double res = 0;      //residue squared
   double slope = 0;    //slope of regression line
   double y_intercept = 0; //y intercept of regression line
   double SUM_Yres = 0; //sum of squared of the discrepancies
   double AVGy = 0;     //mean of y
   double AVGx = 0;     //mean of x
   double Yres = 0;     //squared of the discrepancies
   double Rsqr = 0;     //coefficient of determination

   numPointsToUse = (numPointsToUse <= points.size()) ? numPointsToUse : points.size();

   //calculate various sums
   for (int i = 0; i < numPointsToUse; i++)
   {
      //sum of x
      SUMx = SUMx + points.at(points.size() - 1 - i).centroid.x;
      //sum of y
      SUMy = SUMy + points.at(points.size() - 1 - i).centroid.y;
      //sum of squared x*y
      SUMxy = SUMxy + points.at(points.size() - 1 - i).centroid.x * points.at(points.size() - 1 - i).centroid.y;
      //sum of squared x
      SUMxx = SUMxx + points.at(points.size() - 1 - i).centroid.x * points.at(points.size() - 1 - i).centroid.x;
   }

   //calculate the means of x and y
   AVGy = SUMy / numPointsToUse;
   AVGx = SUMx / numPointsToUse;

   //slope or a1
   slope = (numPointsToUse * SUMxy - SUMx * SUMy) / (numPointsToUse * SUMxx - SUMx*SUMx);

   //y itercept or a0
   y_intercept = AVGy - slope * AVGx;
#if 0
   printf("x mean(AVGx) = %0.5E\n", AVGx);
   printf("y mean(AVGy) = %0.5E\n", AVGy);

   printf ("\n");
   printf ("The linear equation that best fits the given data:\n");
   printf ("       y = %2.8lfx + %2.8f\n", slope, y_intercept);
   printf ("------------------------------------------------------------\n");
   printf ("   Original (x,y)   (y_i - y_avg)^2     (y_i - a_o - a_1*x_i)^2\n");
   printf ("------------------------------------------------------------\n");
#endif
   //calculate squared residues, their sum etc.
   for (int i = 0; i < numPointsToUse; i++)
   {
      //current (y_i - a0 - a1 * x_i)^2
      Yres = pow((points.at(points.size() - 1 - i).centroid.y - y_intercept - (slope * points.at(points.size() - 1 - i).centroid.x)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(points.at(points.size() - 1 - i).centroid.y - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       points.at(points.size() - 1 - i).centroid.x, points.at(points.size() - 1 - i).centroid.y, res, Yres);
#endif
   }

   //calculate r^2 coefficient of determination
   Rsqr = (SUMres - SUM_Yres) / SUMres;
#if 0
   printf("--------------------------------------------------\n");
   printf("Sum of (y_i - y_avg)^2 = %0.5E\t\n", SUMres);
   printf("Sum of (y_i - a_o - a_1*x_i)^2 = %0.5E\t\n", SUM_Yres);
   printf("Standard deviation(St) = %0.5E\n", sqrt(SUMres / (numPointsToUse - 1)));
   printf("Standard error of the estimate(Sr) = %0.5E\t\n", sqrt(SUM_Yres / (numPointsToUse-2)));
   printf("Coefficent of determination(r^2) = %0.5E\t\n", (SUMres - SUM_Yres)/SUMres);
   printf("Correlation coefficient(r) = %0.5E\t\n", sqrt(Rsqr));
#endif
   pair<double,double> result(slope,y_intercept);
   return result;
}
