#ifndef _NETWORK_STREAM_H_
#define _NETWORK_STREAM_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "Blob.h"
#include "Blob.h"

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

static int globalID = 1;

class NetworkStream {

public:
    NetworkStream(const char * networkStream, const char * pluginLocation, int mediaWidth, int mediaHeight) { } // TODO: Callback

    ~NetworkStream() { }

private:

};

#endif

