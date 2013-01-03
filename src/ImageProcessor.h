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
    }

    int processFrame(cv::Mat frame);

    // Can only be set before processing begins
    void setShowFrames(bool d) {
        showFrames = d;
    }

    ~ImageProcessor();

private:
    int frameCount;
    bool useROI;
    bool showFrames;
    cv::Rect roi;
    CarCounter * carCounter;
    IplImage * labelImg;
    IplImage * dstImg;

    cv::BackgroundSubtractorMOG2 bg_model;
};

#endif
