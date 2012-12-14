#ifndef _IMAGE_PROCESSOR_H_
#define _IMAGE_PROCESSOR_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "Blob.h"
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

class ImageProcessor {

public:
    ImageProcessor(const char * imgMaskPath, int mediaWidth, int mediaHeight, CarCounter& c);

    void processFrame(Mat frame);

    ~ImageProcessor();

private:
    int mediaWidth;
    int mediaHeight;
    CarCounter& counter;

};

#endif

