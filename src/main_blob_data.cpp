#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "highgui.h"
#include "CarCounter.h"
#include "DataSourceManager.h"
#include "ImageProcessor.h"

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

ImageProcessor imageProc("/Users/j3bennet/king_st_mask.jpg", 640, 480, NULL);

void processFrame(IplImage* orig);

struct ctx
{
        IplImage* image;
        uchar*    pixels;
};
struct ctx* context;

int frameCount = 0;
int framesProcessed = 0;
long last_time = 0;
void *lock(void *data, void**p_pixels)
{
    //printf("LOCK %d\n", frameCount++);
    *p_pixels = context->pixels;
}


void display(void *data, void *id){
//printf("DISPLAY\n");
}

void unlock(void *data, void *id, void *const *p_pixels){
    //printf("UNLOCK\n");
    struct ctx *ctx = (struct ctx*)data;
    /* VLC just rendered the video, but we can also render stuff */
    uchar *pixels = (uchar*)*p_pixels;
    cv::Mat frame(ctx->image, false);// = Mat(orig, true);
    imageProc.processFrame(frame, true);
    //processFrame(ctx->image);
    cvShowImage("test", ctx->image);
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
    bool displayFrame = false;
    int fps = 0;
// TODO: add fps parameter
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
}

