#ifndef _NETWORK_STREAM_H_
#define _NETWORK_STREAM_H_

#if USE_VLC
#include <vlc/vlc.h>
#endif

#include "ImageProcessor.h"

class NetworkStream {

public:
    NetworkStream(const char * networkStream, ImageProcessor * imgProc, int mediaWidth, int mediaHeight);

    // Does not return
    void startProcessing();

    ~NetworkStream();

private:
#if USE_VLC
    ImageProcessor * imageProcessor;
    libvlc_instance_t * libVlcInstance;
    libvlc_media_player_t* mediaPlayer;
#endif
};

#endif
