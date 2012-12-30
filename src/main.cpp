#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "highgui.h"
#include "CarCounter.h"
#include "DataSourceManager.h"
#include "ImageProcessor.h"
#include "NetworkStream.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <vlc/vlc.h>

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
//#include "LinearRegression.cpp"


#include <sys/time.h>

// Hack to measure how long certain operations take (in ms and us)
// Call profileStart() followed by profileEnd() and it'll print the diff of those call times
timeval profile;
long profile_start, profile_end;
void init();
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
    //printf("To File: %s", line);
    FILE *f = fopen(fileName, "a");
    if (f == NULL) return -1;
    // you might want to check for out-of-disk-space here, too
    fprintf(f, "%s", line);
    fclose(f);
    return 0;
}

// Testing ...
ImageProcessor imageProc(NULL);
NetworkStream networkCamera("http://192.168.1.28/axis-cgi/mjpg/video.cgi?fps=10&nbrofframes=0", &imageProc, 640, 480);


int main(int argc, char* argv[]) {

    // Parse Cmd Line Args
    char * csvData = NULL;
    char * ipCamera = NULL;
    char * videoFile = NULL;
    char * imgMask = NULL;
    char * csvLogFile = NULL;

    int c;
    int dataSources = 0;
    bool displayFrame = false;
    int fps = 0;
// TODO: add fps parameter, w, h params
    while ((c = getopt (argc, argv, "i:o:c:v:m:f:l:dh")) != -1) {
        switch (c)
        {
            case 'i':
                csvData = optarg;
                dataSources++;
                break;
            case 'c':
                ipCamera = optarg;
                dataSources++;
                break;
            case 'v':
                videoFile = optarg;
                dataSources++;
                break;
            case 'm':
                imgMask = optarg;
                break;
            case 'o':
                csvLogFile = optarg;
                break;
            case 'd':
                displayFrame = true;
                break;
            case 'h':
                // TODO: print usage
                return 1;
            default:
                abort();
        }
    }

    if (dataSources > 1) {
        printf("Error: Multiple Data Sources Selected\n");
        return 1;
    }

    DataSourceManager manager;

    // Configure Manager
    if (csvLogFile) {
        manager.setCsvLogFile(csvLogFile);
    }

    if (imgMask) {
        manager.setImgMask(imgMask);
    }
#if 0
    // Process Data or Stream from Camera
    if (csvData) {
        manager.processCsvFile(csvData);
    } else if (ipCamera) {
        manager.processIpCamera(ipCamera);
    } else if (videoFile) {
        manager.processVideoFile(videoFile);
    }
#endif

    cv::Rect roi(265, 230, 375, 225);
    imageProc.setROI(roi);

    networkCamera.startProcessing();
}
