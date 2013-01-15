#ifndef _IMAGE_PROCESSOR_H_
#define _IMAGE_PROCESSOR_H_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include "Blob.h"
#include "CarCounter.h"

class ImageProcessor {

public:
    ImageProcessor(CarCounter * c);

    void setCrop(int x, int y, int w, int h) {
        roi = cv::Rect(x, y, w, h);
        useROI = true;
        printf("Setting Crop x=%d y=%d w=%d h=%d\n", x, y, w, h);
    }

    void setJpegDumpPath(char *path) {
        jpegDumpPath = path;
    }

    int processFrame(cv::Mat& frame, long time);

    // Can only be set before processing begins
    void setShowFrames(bool d);

    void processVideoFile(const char * path);

    // Returns time in ms  // TODO: put in utils class?
    static long getTime();

    ~ImageProcessor();

private:
    int frameCount;
    bool useROI;
    bool showFrames;
    cv::Rect roi;
    CarCounter * carCounter;
    IplImage * labelImg;
    IplImage * dstImg;

    char *jpegDumpPath;

    cv::BackgroundSubtractorMOG2 bg_model;

    // Stats
    long tStart;
    long tmin, tmax, tavg, ttotal, tfps;
    int lastStatsPrinted;
};

#endif
