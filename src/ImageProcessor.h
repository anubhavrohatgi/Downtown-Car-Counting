#ifndef _IMAGE_PROCESSOR_H_
#define _IMAGE_PROCESSOR_H_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include "Blob.h"
#include "CarCounter.h"

class ImageProcessor {

public:
    ImageProcessor(CarCounter * c, bool showFrames=true);

    void setROI(cv::Rect r) {
        roi = r;
        useROI = true;
    }

    int processFrame(cv::Mat frame);

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
