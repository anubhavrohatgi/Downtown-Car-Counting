#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "highgui.h"
#include "CarCounter.h"
#include "CarCountManager.h"

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
    printf("To File: %s", line);
    FILE *f = fopen(fileName, "a");
    if (f == NULL) return -1;
    // you might want to check for out-of-disk-space here, too
    fprintf(f, "%s", line);
    fclose(f);
    return 0;
}

void processFrame(IplImage* orig);

struct ctx
{
        IplImage* image;
        uchar*    pixels;
};
struct ctx* context;

int frameCount = 0;

void *lock(void *data, void**p_pixels)
{
printf("LOCK %d\n", frameCount++);
*p_pixels = context->pixels;

}


void display(void *data, void *id){
printf("DISPLAY\n");
}

void unlock(void *data, void *id, void *const *p_pixels){
printf("UNLOCK\n");
struct ctx *ctx = (struct ctx*)data;
        /* VLC just rendered the video, but we can also render stuff */
        uchar *pixels = (uchar*)*p_pixels;
        processFrame(ctx->image);
        //cvShowImage("test", ctx->image);

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


// VLC STUFF
    libvlc_instance_t * inst;
             libvlc_media_player_t *mp;
             libvlc_media_t *m;

             const char * const vlc_args[] = {
             "--no-media-library",
             "--reset-plugins-cache",
             "--no-stats",
             "--no-osd",
             "--no-video-title-show",
             "--plugin-path=/usr/local/lib/"};

             const char * const vlc2_args[] = {
             "--plugin-path=/usr/local/lib"};

             /* Load the VLC engine */
             //inst = libvlc_new (6, vlc_args);
    #if 1
             /* Create a new item */
    //         m = libvlc_media_new_path (inst, "http://mycool.movie.com/test.mov");
    printf("B4\n");
             //int a = libvlc_vlm_add_broadcast(inst, "mybroadcast", "http://192.168.1.28/axis-cgi/mjpg/video.cgi?fps=30&nbrofframes=0", "#transcode{vcodec=h264,vb=0,scale=0,acodec=mp4a,ab=128,channels=2,samplerate=44100}:rtp{sdp=rtsp://:5544/}", 0, NULL, TRUE, 0);


    init();
            cvNamedWindow("test", CV_WINDOW_AUTOSIZE);
            libvlc_media_t* media = NULL;
            libvlc_media_player_t* mediaPlayer = NULL;
            char const* vlc_argv[] = {"--plugin-path", "/usr/local/lib"};
            libvlc_instance_t* instance = libvlc_new(6,vlc_args);
            media = libvlc_media_new_path(instance, "http://192.168.1.28/axis-cgi/mjpg/video.cgi?fps=10&nbrofframes=0");
            mediaPlayer = libvlc_media_player_new_from_media(media);
            libvlc_media_release(media);

            //media = libvlc_media_new_path(instance, "http://137.82.120.10:8008");

            context = ( struct ctx* )malloc( sizeof( *context ) );
            context->image = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);
        context->pixels = (unsigned char *)context->image->imageData;

            //libvlc_media_player_set_media( mediaPlayer, media);
            libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, context);
            libvlc_video_set_format(mediaPlayer, "RV32", 640, 480, 640*4);
            libvlc_media_player_play(mediaPlayer);


            while(1)
            {
               cvWaitKey(10);

            }
             /* Create a media player playing environement */
             mp = libvlc_media_player_new_from_media (m);

             /* No need to keep the media now */
             libvlc_media_release (m);

         #if 0
             /* This is a non working code that show how to hooks into a window,
              * if we have a window around */
              libvlc_media_player_set_xdrawable (mp, xdrawable);
             /* or on windows */
              libvlc_media_player_set_hwnd (mp, hwnd);
             /* or on mac os */
              libvlc_media_player_set_nsobject (mp, view);
          #endif

             /* play the media_player */
             libvlc_media_player_play (mp);

             sleep (10); /* Let it play a bit */

             /* Stop playing */
             libvlc_media_player_stop (mp);

             /* Free the media_player */
             libvlc_media_player_release (mp);

             libvlc_release (inst);
    #endif

#if 1


#endif
}

Mat aframe, foreground, image;
BackgroundSubtractorMOG2 mog;

cv::Mat frame;
cv::Mat back;
cv::Mat fore;
cv::BackgroundSubtractorMOG2 bg(0, 7, false);

Mat eastbound_mask;
Mat westbound_mask;
Mat bgImg;

int fps = 5;
int totalFrame = 1000;

CvBlobs blobs;

BackgroundSubtractorMOG2 bg_model;//(100, 3, 0.3, 5);
Mat img, fgmask, fgimg;

CvSize vga;
int key;

void init()
{
    namedWindow("Capture ", CV_WINDOW_AUTOSIZE);
    namedWindow("Background", CV_WINDOW_AUTOSIZE);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;

    //CvCapture* capture = cvCaptureFromAVI(NULL);

    eastbound_mask = imread("/Users/j3bennet/king_st_eastbound_mask.jpg",CV_LOAD_IMAGE_ANYCOLOR); // Read the file
    westbound_mask = imread("/Users/j3bennet/king_st_westbound_mask.jpg",CV_LOAD_IMAGE_ANYCOLOR); // Read the file

    //Mat eastbound_mask = imread("/Users/j3bennet/eastbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat eastbound_mask = imread("/Users/j3bennet/westbound_chunk.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat westbound_mask = imread("/Users/j3bennet/king_st_westbound_mask.jpg", CV_LOAD_IMAGE_ANYCOLOR);   // Read the file
    //Mat bgImgGrey = imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
    bgImg =
            imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_ANYCOLOR); // Read the file // TODO: WTF is this ?


    /* get fps, needed to set the delay */
#if 0
    int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
    int totalFrame = (int) cvGetCaptureProperty(capture,
            CV_CAP_PROP_FRAME_COUNT);
    printf("FPS: %d  TOTAL FRAMES %d\n", fps, totalFrame);
#endif

    /* display video */
    //    cvNamedWindow( "video", 0 );
    //    cvNamedWindow("foreground", 0);

    //Mat bg_filtered;
    //cv::add(bgImgGrey, mask, bg_filtered);

#if 0
    namedWindow("image", CV_WINDOW_NORMAL);
    namedWindow("foreground mask", CV_WINDOW_NORMAL);
    namedWindow("foreground image", CV_WINDOW_NORMAL);
    namedWindow("mean background image", CV_WINDOW_NORMAL);
#endif


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

    vga.height = 480;
    vga.width = 640;

    namedWindow("video", CV_WINDOW_NORMAL);
    namedWindow("fg-before", CV_WINDOW_NORMAL);
    namedWindow("fg-after", CV_WINDOW_NORMAL);
    namedWindow("fgmask", CV_WINDOW_NORMAL);
}

void processFrame(IplImage* orig)
{
    cv::Mat frame(orig, false);// = Mat(orig, true);

    // Mask Image
    Mat masked;
    try {
        cvtColor(frame, frame, CV_BGR2RGB); // TODO: fix innefficiency
        cv::add(frame, eastbound_mask, masked);
        cv::imshow("video", frame);
    #if 1
        // Background Subtraction
        if (fgimg.empty()) {
            fgimg.create(img.size(), img.type());
        }

        //update the model
        bool update_bg_model = true; //(blobs.size() > 0 && frameCount > 300) ? false : true;
        bg_model(masked, fgmask, update_bg_model ? -1 : 0);

        fgimg = Scalar::all(0);
        masked.copyTo(fgimg, fgmask);
        //filtered.copyTo(fgimg, fgimg);

        Mat bgimg;
        bg_model.getBackgroundImage(bgimg);

        cvtColor(fgimg, fgimg, CV_BGR2GRAY);
        //threshold(fgimg, fgimg, 100, 255, 3);
        //fgimg = GetThresholdedImage(fgimg);
        IplImage filtered_img = fgimg;

    #if 1
        IplImage *labelImg = cvCreateImage(vga, IPL_DEPTH_LABEL, 1);
        IplImage *dstImg = cvCreateImage(vga, IPL_DEPTH_8U, 3);

        unsigned int result = cvLabel(&filtered_img, labelImg, blobs);

        cvFilterByArea(blobs, 100, 2000);

        for (CvBlobs::iterator it = blobs.begin(); it != blobs.end(); ++it) {
                CvBlob blob = *(it->second);
                if (blob.area > 0) {
                    char buf[120];
                    sprintf(buf, "%d,%f,%f,%d\n", frameCount, blob.centroid.x, blob.centroid.y, (int)blob.area);
                    write_to_file("/Users/j3bennet/r1217.csv", buf);
                }
        }
        //cvUpdateTracks(blobs, tracks, 10., 5);


        int newROs = 0;

        //printf("%d\n", blobs.size());



        cvRenderBlobs(labelImg, blobs, &filtered_img, dstImg);
        //cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_BOUNDING_BOX);
        //cvRenderTracks(tracks, dstImg, dstImg, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_LOG);
        //cvRenderTracks(tracks, dstImg, dstImg, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
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
    #if 1
        key = cvWaitKey(1);
         if (key == 'p') {
             do {
                 key = cvWaitKey(1);
             } while (key != 'p');
         }
    #endif
    #endif

    } catch (cv::Exception& e) {
        printf("Caught Exception: %s\n", e.what());
    }
}
