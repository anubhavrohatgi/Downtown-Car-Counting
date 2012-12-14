#include "ImageProcessor.h"

using namespace cv;

ImageProcessor::ImageProcessor(const char * imgMaskPath, int mediaWidth, int mediaHeight, CarCounter& c) :
    mediaWidth(mediaWidth),
    mediaHeight(mediaHeight),
    counter(c)
{

}

int processFrame(Mat frame)
{

}

ImageProcessor::~ImageProcessor()
{

}

