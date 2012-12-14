#ifndef _NETWORK_STREAM_H_
#define _NETWORK_STREAM_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include <vlc/vlc.h>

#include "Blob.h"
#include "ImageProcessor.h"

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

class NetworkStream {

public:
    NetworkStream(const char * networkStream, ImageProcessor * imgProc, int mediaWidth, int mediaHeight);

    // Does not return
    void startProcessing();

    ImageProcessor * getImageProcessor()
    {
        return imageProcessor;
    }

    ~NetworkStream();

private:
    ImageProcessor * imageProcessor;
    libvlc_instance_t * libVlcInstance;
    libvlc_media_player_t* mediaPlayer;
};

#endif

