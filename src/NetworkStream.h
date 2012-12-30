#ifndef _NETWORK_STREAM_H_
#define _NETWORK_STREAM_H_

#include <vlc/vlc.h>

#include "ImageProcessor.h"

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
