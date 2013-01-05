#if 1
#include "DataSourceManager.h"
#include "ImageProcessor.h"

void printUsage(const char * name) {
    printf("usage: %s [...]\n" \
            "\nData Sources:\n"\
            "   -i <input_csv>\n"\
            "   -c <libvcl_connection_string> # EXAMPLE: -c http://192.168.1.28/axis-cgi/mjpg/video.cgi?fps=10&nbrofframes=0\n" \
            "   -v <video_path>\n" \
            "\nOutput\n" \
            "   -o <output_csv_log> \n" \
            "   -d # specifies if frames should be displayed while processing (valid with -c, -v)\n" \
            "\nCropping (Optional, valid with -c and -v options)\n" \
            "   -x <crop_x>\n" \
            "   -y <crop_y>\n" \
            "   -l <crop_length>\n" \
            "   -t <crop_height>\n" \
            "\nMisc\n"
            "   -w <media_width> # must be specified if using -c \n" \
            "   -h <media_height> # must be specified if using -c \n" \
            "   -? # Usage Statement\n" \
            , name);
}

int main(int argc, char* argv[]) {
    // Parse Cmd Line Args
    char * csvData = NULL;
    char * ipCamera = NULL;
    char * videoFile = NULL;
    char * csvLogFile = NULL;

    // Media dimensions
    int w=0, h=0;

    // Optional cropping values
    int x=0, y=0, l=0, t=0;

    int c;
    int dataSources = 0;
    bool displayFrames = false;
    int fps = 0;
// TODO: add fps parameter,
    while ((c = getopt (argc, argv, "i:o:c:v:m:f:l:dx:y:l:t:w:h:?")) != -1) {
        switch (c)
        {
        // Data Sources
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
            case 'w':
                w = atoi(optarg);
                break;
            case 'h':
                h = atoi(optarg);
                break;
        // Cropping
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case 'l':
                l = atoi(optarg);
                break;
            case 't':
                t = atoi(optarg);
                break;
        // Outputs
            case 'o':
                csvLogFile = optarg;
                break;
            case 'd':
                displayFrames = true;
                break;
            case '?':
                printUsage(argv[0]);
            default:
                return 1;
        }
    }

    // Filter out invalid combinations
    if (argc == 1) {
        printUsage(argv[0]);
        return 1;
    } else if (dataSources > 1) {
        printf("Error: Multiple Data Sources Selected\n");
        return 1;
    } else if (dataSources == 0) {
        printf("Error: No Data Sources Selected\n");
        return 1;
    } else if (csvData && (displayFrames || w || h)) {
        printf("Invalid option specified.\n");
        return 1;
    } else if (ipCamera && (!w || !h)) {
        printf("Must specify media dimensions -w, -h.\n");
        return 1;
    }

    DataSourceManager manager;

    // Configure Manager
    if (csvLogFile) {
        manager.setCsvLogFile(csvLogFile);
    }

    if (l && t) {
        manager.getImageProcessor().setCrop(x, y, l, t);
    }

    // Process Data or Stream from Camera
    if (csvData) {
        manager.processCsvFile(csvData);
    } else {
        printf("Setting Frames to be Displayed: %d\n", displayFrames);
        manager.getImageProcessor().setShowFrames(displayFrames);
        if (ipCamera) {
            manager.processIpCamera(ipCamera, w, h);
        } else if (videoFile) {
            manager.processVideoFile(videoFile);
        }
    }
}
#else

#include <iostream>
#include <vector>

//#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>
//#include <opencv2/highgui/highgui_c.h>

using namespace cv;
using namespace std;


struct mouse_info_struct { int x,y; };
struct mouse_info_struct mouse_info = {-1,-1}, last_mouse;

vector<Point> mousev,kalmanv;

void on_mouse(int event, int x, int y, int flags, void* param) {
    //if (event == CV_EVENT_LBUTTONUP)
    {
        last_mouse = mouse_info;
        mouse_info.x = x;
        mouse_info.y = y;

//      cout << "got mouse " << x <<","<< y <<endl;
    }
}

int main (int argc, char * const argv[]) {
    Mat img(500, 500, CV_8UC3);
    KalmanFilter xyFilter(4, 2, 0);
    Mat_<float> state(4, 1); /* (x, y, Vx, Vy) */
    Mat processNoise(4, 1, CV_32F);
    Mat_<float> measurement(2,1);
    measurement.setTo(Scalar(0));
    char code = (char)-1;

    namedWindow("mouse kalman");
    setMouseCallback("mouse kalman", on_mouse, 0);

    for(;;)
    {
        if (mouse_info.x < 0 || mouse_info.y < 0) {
            imshow("mouse kalman", img);
            waitKey(30);
            continue;
        }
        xyFilter.statePre.at<float>(0) = mouse_info.x;
        xyFilter.statePre.at<float>(1) = mouse_info.y;
        xyFilter.statePre.at<float>(2) = 0;
        xyFilter.statePre.at<float>(3) = 0;
        xyFilter.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,0,0,   0,1,0,0,  0,0,1,0,  0,0,0,1);

        setIdentity(xyFilter.measurementMatrix);
        setIdentity(xyFilter.processNoiseCov, Scalar::all(1e-4));
        setIdentity(xyFilter.measurementNoiseCov, Scalar::all(1e-1));
        setIdentity(xyFilter.errorCovPost, Scalar::all(.1));

        mousev.clear();
        kalmanv.clear();
        //randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));

        for(;;)
        {
//            Point statePt(state(0),state(1));

            Mat prediction = xyFilter.predict();
            Point predictPt(prediction.at<float>(0),prediction.at<float>(1));

            measurement(0) = mouse_info.x;
            measurement(1) = mouse_info.y;

            Point measPt(measurement(0),measurement(1));
            mousev.push_back(measPt);
            // generate measurement
            //measurement += KF.measurementMatrix*state;

            Mat estimated = xyFilter.correct(measurement);
            Point statePt(estimated.at<float>(0),estimated.at<float>(1));
            kalmanv.push_back(statePt);

            // plot points
#define drawCross( center, color, d )                                 \
line( img, Point( center.x - d, center.y - d ),                \
Point( center.x + d, center.y + d ), color, 2, CV_AA, 0); \
line( img, Point( center.x + d, center.y - d ),                \
Point( center.x - d, center.y + d ), color, 2, CV_AA, 0 )

            img = Scalar::all(0);
            drawCross( statePt, Scalar(255,255,255), 5 );
            drawCross( measPt, Scalar(0,0,255), 5 );
//            drawCross( predictPt, Scalar(0,255,0), 3 );
//          line( img, statePt, measPt, Scalar(0,0,255), 3, CV_AA, 0 );
//          line( img, statePt, predictPt, Scalar(0,255,255), 3, CV_AA, 0 );

            for (int i = 0; i < mousev.size()-1; i++) {
                line(img, mousev[i], mousev[i+1], Scalar(255,255,0), 1);
            }
            for (int i = 0; i < kalmanv.size()-1; i++) {
                line(img, kalmanv[i], kalmanv[i+1], Scalar(0,255,0), 1);
            }


//            randn( processNoise, Scalar(0), Scalar::all(sqrt(KF.processNoiseCov.at<float>(0, 0))));
//            state = KF.transitionMatrix*state + processNoise;

            imshow( "mouse kalman", img );
            code = (char)waitKey(100);

            if( code > 0 )
                break;
        }
        if( code == 27 || code == 'q' || code == 'Q' )
            break;
    }

    return 0;
}
#endif
