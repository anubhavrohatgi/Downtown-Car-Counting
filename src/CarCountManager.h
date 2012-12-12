#ifndef _CAR_COUNT_MANAGER_H_
#define _CAR_COUNT_MANAGER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "CarCounter.h"
#include "CarCountManager.h"

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

using namespace std;

class CarCountManager {

public:
    CarCountManager();
    ~CarCountManager();

    void setCsvLogFile(const char * path);
    void setImgMask(const char * path);

    int processVideoFile(const char * path);

    // CSV File is expected with at least 4 columns
    // frameNumber, blob x, blob y, blob size
    int processCsvFile(const char * path);

    int processIpCamera(const char * ip);

private:
    CarCounter counter;

    bool logToCsv;
    bool useImgMask;
    const char * csvLogFile;
    const char * imgMaskFile;
};

#endif
