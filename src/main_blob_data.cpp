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


// FIXME
//#include "LinearRegression.cpp"


#include <sys/time.h>

// Hack to measure how long certain operations take (in ms and us)
// Call profileStart() followed by profileEnd() and it'll print the diff of those call times
timeval profile;
long profile_start, profile_end;

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
    printf("To File: %s", line);
    FILE *f = fopen(fileName, "a");
    if (f == NULL) return -1;
    // you might want to check for out-of-disk-space here, too
    fprintf(f, "%s", line);
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]) {

    // Parse Cmd Line Args
    char * csvData = NULL;
    char * ipCamera = NULL;
    char * videoFile = NULL;
    char * imgMask = NULL;
    char * csvLogFile = NULL;

    int c;
    int dataSources = 0;

    while ((c = getopt (argc, argv, "d:i:v:m:f:l:h")) != -1) {
        switch (c)
        {
            case 'd':
                csvData = optarg;
                dataSources++;
                break;
            case 'i':
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
            case 'l':
                csvLogFile = optarg;
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

    CarCountManager manager;

    // Configure Manager
    if (csvLogFile) {
        manager.setCsvLogFile(csvLogFile);
    }

    if (imgMask) {
        manager.setImgMask(imgMask);
    }
#if 1
    // Process Data or Stream from Camera
    if (csvData) {
        manager.processCsvFile(csvData);
    } else if (ipCamera) {
        manager.processIpCamera(ipCamera);
    } else if (videoFile) {
        manager.processVideoFile(videoFile);
    }
#endif
#if 0
    if (argc < 2) {
        printf("Usage: %s PATH_TO_RAW_AVI_VIDEO_FILE [IMAGE_MASK_JPG_OR_OTHER_FORMAT]", argv[0]);
        return 0;
    }
    // TODO: use string?
    char * video_filename = argv[1];

    bool useImgMask = false;
    if (argc > 2) {
        useImgMask = true;
    }
    //     //sept9_raw60

    char buf[80];
    time_t t;
    time(&t);
    sprintf(buf, "%s-data(%d).csv", video_filename, t);
    char * data_filename = buf;
    //filename = "/Users/j3bennet/afternoon_raw_cut.avi";

    CvCapture* capture = cvCaptureFromAVI(video_filename);
    //CvCapture *capture = cvCreateFileCapture_FFMPEG("http://192.168.1.28/mjpg/video.mjpg?resolution=640x480&req_fps=10&.mjpg");
    //CvCapture *capture=cvCaptureFromFile("http://192.168.1.28/axis-cgi/mjpg/video.cgi?resolution=640x480&req_fps=10&.mjpg");


    namedWindow("Capture ", CV_WINDOW_AUTOSIZE);
    namedWindow("Background", CV_WINDOW_AUTOSIZE);
    Mat aframe, foreground, image;
    BackgroundSubtractorMOG2 mog;

    //capture = cvCaptureFromCAM(0); // capture from video device #0
    //Capturing a frame:
    //IplImage* img = 0;

    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    cv::BackgroundSubtractorMOG2 bg(0, 7, false);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;

    std::vector<std::vector<cv::Point> > contours;

    Mat eastbound_mask;
    if (useImgMask) {
        eastbound_mask = imread(argv[2],CV_LOAD_IMAGE_ANYCOLOR); // Read the file
    }
    //Mat eastbound_mask = imread("/Users/j3bennet/eastbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat eastbound_mask = imread("/Users/j3bennet/westbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat westbound_mask = imread("/Users/j3bennet/king_st_westbound_mask.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat bgImgGrey = imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
    Mat bgImg =
            imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_ANYCOLOR); // Read the file


    /* get fps, needed to set the delay */
    int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
    int totalFrame = (int) cvGetCaptureProperty(capture,
            CV_CAP_PROP_FRAME_COUNT);

    /* display video */
    //    cvNamedWindow( "video", 0 );
    //    cvNamedWindow("foreground", 0);

    IplImage *orig;
    int key;

    //Mat bg_filtered;
    //cv::add(bgImgGrey, mask, bg_filtered);

    CvBlobs blobs;
    CvTracks tracks;

#if 0
    namedWindow("image", CV_WINDOW_NORMAL);
    namedWindow("foreground mask", CV_WINDOW_NORMAL);
    namedWindow("foreground image", CV_WINDOW_NORMAL);
    namedWindow("mean background image", CV_WINDOW_NORMAL);
#endif

    BackgroundSubtractorMOG2 bg_model;//(100, 3, 0.3, 5);
    Mat img, fgmask, fgimg;

    //Mat bg_filtered;
    //cv::add(bgImg, eastbound_mask, bg_filtered);
#if 0
    //update the model
    for (int i = 0; i < 100; i++) {
        bool update_bg_model = true;
        bg_model(bg_filtered, fgmask, update_bg_model ? -1 : 0);
        fgimg = Scalar::all(0);
        bg_filtered.copyTo(fgimg, fgmask);

    }
#endif

    int frameCount = 0;

    // Eastbound Params
    //Rect east_traffic_boundaries(275, 300, (610-275), (410-300));
    //double expectedPathSlope = 0.2; // From experimentation
    //CarCounter counter(50, &east_traffic_boundaries, true, expectedPathSlope);

    // Westbound Params
    Rect westTrafficBoundaries(260, 250, (600-260), (350-250));
    double expectedPathSlope = 0.2; // From experimentation
    CarCounter counter();

    CvSize vga;
    vga.height = 480;
    vga.width = 640;

    timeval time;

    gettimeofday(&time, NULL);
    long last_run = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    long start_time = last_run;
    int min = 999, sum = 0, max = 0;

    namedWindow("video", CV_WINDOW_NORMAL);
    namedWindow("fg-before", CV_WINDOW_NORMAL);
    namedWindow("fg-after", CV_WINDOW_NORMAL);
    namedWindow("fgmask", CV_WINDOW_NORMAL);

    int actionFrameCount = 0;

    // Don't start processing immeditely, need time to learn background and such
    bool stateProcessCounts = false;
    static int BG_LEARNING_TIME_MS = 1000;

    int bgLearningFramesReqd = (fps * 1000) / BG_LEARNING_TIME_MS;
    int sequentialLearningFrames = 0;

    while (key != 'q' && frameCount < totalFrame) {
        /* get a frame */
        orig = cvQueryFrame(capture);

        cv::Mat frame(orig, true);// = Mat(orig, true);
        frameCount++;

        // Mask Image
        Mat masked;
        try {

#if 1

#if 0
            // SET ROI
            cv::Rect roi = cv::Rect((img2.cols - img1.cols)/2,(img2.rows - img1.rows)/2,img1.cols,img1.rows);

            cv::Mat roiImg;
            roiImg = img2(roi);

            img1.copyTo(roiImg);
#endif
            if (useImgMask) {
                cv::add(frame, eastbound_mask, masked);
            } else {
                frame.copyTo(masked); // TODO: fix up these names and such
            }
            cv::imshow("video", frame);

            // Background Subtraction
            if (fgimg.empty())
                fgimg.create(img.size(), img.type());

            //update the model
            bool update_bg_model = true; //(blobs.size() > 0 && frameCount > 300) ? false : true;
            bg_model(masked, fgmask, update_bg_model ? -1 : 0);

            fgimg = Scalar::all(0);
            masked.copyTo(fgimg, fgmask);
            //filtered.copyTo(fgimg, fgimg);

            Mat bgimg;
            bg_model.getBackgroundImage(bgimg);

            //imshow("fg-before", fgimg);

            //cv::erode(fgimg,fgimg,cv::Mat()); // doesn't seem to help much - makes black zones
            //cv::dilate(fgimg,fgimg,cv::Mat()); // makes img more pixelated

#if 0
            // Doesn't seem to help much
            // Pyramid Transform
            CvSize sz;
            sz.height = 480/2;
            sz.width = 640/2;
            Mat downSampled;
            downSampled.create(sz, fgimg.type());
            pyrDown(fgimg, downSampled,downSampled.size());
            //void cvPyrDown( const CvArr* src, CvArr* dst, int filter=CV_GAUSSIAN_5x5);

            // Smoothing
            // Soft smoothing
            Mat smoothed;
            smoothed.create(fgimg.size(), fgimg.type());
            //cvtColor(fgimg, fgimg, CV_BGR2HSV);
            bilateralFilter(fgimg, smoothed, 5, 20,20);
#endif

            //               cvtColor(fgimg, fgimg, CV_BGR2HSV);
            //               cvCvtColor(img, imgHSV, CV_BGR2HSV); // convert the coming image from the camera from RGB format to HSV (Hue, Saturation, Value)
            ///               IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1); //hold the thresholded image
            //               cvSmooth(imgHSV, imgHSV, CV_GAUSSIAN, 11, 11);//smooth the image to remove the noise from the image

            cvtColor(fgimg, fgimg, CV_BGR2GRAY);
            //threshold(fgimg, fgimg, 100, 255, 3);
            //fgimg = GetThresholdedImage(fgimg);
            IplImage filtered_img = fgimg;

#if 1
            IplImage *labelImg = cvCreateImage(vga, IPL_DEPTH_LABEL, 1);
            IplImage *dstImg = cvCreateImage(vga, IPL_DEPTH_8U, 3);

            unsigned int result = cvLabel(&filtered_img, labelImg, blobs);

            cvFilterByArea(blobs, 100, 2000);


            cvUpdateTracks(blobs, tracks, 20, 1);

            for (CvTracks::iterator it = tracks.begin(); it != tracks.end(); ++it) {
                CvTrack track = *(it->second);
                char buf[120];
                double area = (track.maxx - track.minx) * (track.maxy - track.miny);
                sprintf(buf, "%d,%f,%f\n", frameCount, track.centroid.x, area);
                //printf("%s\n", buf);
               //write_to_file("tracks459.csv", buf);
            }

            for (CvBlobs::iterator it = blobs.begin(); it != blobs.end(); ++it) {
                    CvBlob blob = *(it->second);
                    if (blob.area > 0) {
                        char buf[120];
                        sprintf(buf, "%d,%f,%f,%d\n", frameCount, blob.centroid.x, blob.centroid.y, (int)blob.area);
                        write_to_file("/Users/j3bennet/blobs1204-10fps-east.csv", buf);
                    }
            }
            //cvUpdateTracks(blobs, tracks, 10., 5);


            int newROs = 0;

            //printf("%d\n", blobs.size());

            if (!stateProcessCounts) {
                if (blobs.size() == 0) {
                    sequentialLearningFrames++;
                } else {
                    sequentialLearningFrames = 0;
                }

                if (sequentialLearningFrames == bgLearningFramesReqd) {
                    stateProcessCounts = true;
                    printf("BG Learning Complete ... starting to count cars!\n");
                }
            }

            if (stateProcessCounts) {
                //newROs = counter.updateStats(tracks);
                //printf("AVG SPEED %.2f\n", counter.getAvgSpeed(2));
            }

            if (totalFrame == frameCount) {
                // Video clip is done, let everything timeout ...
                printf("Starting timeout sequence ...\n");
                tracks.clear();
                for (int i = 0; i < 50; i++) {
                    //newROs = counter.updateStats(tracks);
                }
            }

            //CvLabel index = cvLargestBlob(blobs);
            //printf("%d,%d,%d,%d\n",frameCount, blobs.size(), index, index ? blobs.at(index)->area : 0);

            cvRenderBlobs(labelImg, blobs, &filtered_img, dstImg);
            //cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_BOUNDING_BOX);
            //cvRenderTracks(tracks, dstImg, dstImg, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_LOG);
            //cvRenderTracks(tracks, dstImg, dstImg, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);


            for (CvBlobs::iterator it = blobs.begin(); it != blobs.end(); ++it) {

                        CvBlob blob = *(it->second);
            #if 0
                        cout << "Blob #" << it->second->label << ": Area="
                                << it->second->area << ", Centroid=("
                                << it->second->centroid.x << ", "
                                << it->second->centroid.y << ")" << endl;
                        printf("BOUNDS %d %d %d %d\n", blob.minx, blob.maxx, blob.miny, blob.maxy);
            #endif
                    }

            if (blobs.size() > 0) {
                actionFrameCount++;
            }
#endif
            cvShowImage("dstImg", dstImg);
            //imshow("fg-after", fgimg);
            cvReleaseImage(&labelImg);
            cvReleaseImage(&dstImg);
            //imshow("fg-after", filtered_img);
            //imshow("fgmask", fgmask);

            //imshow("image", frame);
            //imshow("foreground mask", fgmask);
            //imshow("foreground image", fgimg);
            if (!bgimg.empty()) {
                //imshow("mean background image", bgimg );
            }
#endif
#if 1
            key = cvWaitKey(1);
             if (key == 'p') {
                 do {
                     key = cvWaitKey(1);
                 } while (key != 'p');
             }
#endif

        } catch (cv::Exception& e) {
            printf("Caught Exception: %s\n", e.what());
        }

        // Keep track of timing
        // ms timing on Mac and Windows: http://brian.pontarelli.com/2009/01/05/getting-the-current-system-time-in-milliseconds-with-c/
        gettimeofday(&time, NULL);
        long this_run = (time.tv_sec * 1000) + (time.tv_usec / 1000);
        int processingTime = this_run - last_run;
        sum += processingTime;
        min = (processingTime < min) ? processingTime : min;
        max = (processingTime > max) ? processingTime : max;
        last_run = this_run;

        // Needed to display a video image.  performance: fps from 27 to 22
        //printf("Frame %d/%d (min,avg,max) (%d,%d,%d) ms fps=%d\n", frameCount, totalFrame, min, sum/frameCount, max, 1000*frameCount/sum);
    }

#if 0
    //Distance::ElementType d;
    Mat A(500, 2, DataType<float>::type);

    cvflann::Matrix<Distance> m;

    cvflann::flann_distance_t

    //Mat points<Distance::ElementType>;
    cvflann::hierarchicalClustering
#endif
    printf("Frame %d (min,avg,max) (%d,%d,%d) ms fps=%d\n", frameCount, min, sum/frameCount, max, 1000*frameCount/sum);

    /* free memory */
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
#endif
}

