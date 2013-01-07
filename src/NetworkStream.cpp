#include "NetworkStream.h"

static ImageProcessor * imProc = NULL;

struct ctx
{
        IplImage* image;
        uchar*    pixels;
};
static struct ctx* context;

static void * lock(void *data, void**p_pixels)
{
    //printf("LOCK %d\n", 3);
    *p_pixels = context->pixels;
}


static void display(void *data, void *id){
//printf("DISPLAY\n");
}

static void unlock(void *data, void *id, void *const *p_pixels){
    //printf("UNLOCK\n");
    struct ctx *ctx = (struct ctx*)data;
    /* VLC just rendered the video, but we can also render stuff */
    uchar *pixels = (uchar*)*p_pixels;
    cv::Mat frame(ctx->image, false);// = Mat(orig, true);
    if (imProc) {
        imProc->processFrame(frame, ImageProcessor::getTime());
    } else {
        printf("No IMG proc\n");
    }
    //processFrame(ctx->image);
    //cvShowImage("test", ctx->image);
}

NetworkStream::NetworkStream(const char * networkStream, ImageProcessor * proc, int mediaWidth, int mediaHeight) :
    imageProcessor(proc),
    libVlcInstance(NULL),
    mediaPlayer(NULL)
{
    // Hack since callback functions require static instance.
    imProc = imageProcessor;

    const char * const vlc_args[] = {
    "--no-media-library",
    "--reset-plugins-cache",
    "--no-stats",
    "--no-osd",
    "--no-video-title-show",
    "--plugin-path=/usr/local/lib/"};
    printf("Starting Network Stream: %dx%d\n", mediaWidth, mediaHeight);
    libVlcInstance = libvlc_new(6,vlc_args);

    libvlc_media_t * media = libvlc_media_new_path(libVlcInstance, networkStream);

    mediaPlayer = libvlc_media_player_new_from_media(media);

    libvlc_media_release(media);


    context = ( struct ctx* )malloc( sizeof( *context ) );
    context->image = cvCreateImage(cvSize(mediaWidth, mediaHeight), IPL_DEPTH_8U, 4);
    context->pixels = (unsigned char *)context->image->imageData;

    //libvlc_media_player_set_media( mediaPlayer, media);
    libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, context);
    libvlc_video_set_format(mediaPlayer, "RV32", mediaWidth, mediaHeight, mediaWidth*4);
}

// Does not return
void NetworkStream::startProcessing()
{
    libvlc_media_player_play(mediaPlayer);

    while(1)
    {
        cvWaitKey(10);
    }
}


NetworkStream::~NetworkStream()
{
    /* No need to keep the media now */
    libvlc_media_player_release (mediaPlayer);

    libvlc_release(libVlcInstance);

    free(context);
}
