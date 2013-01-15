#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "DataSourceManager.h"
#include "ImageProcessor.h"

void printUsage(const char * name) {
    printf("usage: %s [...]\n" \
            "\nData Sources:\n"\
            "   -i <input_csv>\n"\
            "   -c <libvcl_connection_string> # EXAMPLE: -c http://192.168.1.28/axis-cgi/mjpg/video.cgi?fps=10&nbrofframes=0\n" \
            "   -v <video_path>\n" \
            "\nOutput\n" \
            "   -b <output_csv_blobs_log> \n" \
            "   -o <output_csv_objects_detected_log> \n" \
            "   -d # specifies if frames should be displayed while processing (valid with -c, -v)\n" \
            "   -j <path> # dumps jpeg frames to directory for making into a movie or debug purposes\n" \
            "             # (format ImageA[frameCount mod 1000].jpg and imageB[frameCount mod 1000].jpg)\n" \
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

DataSourceManager *dataManager = NULL;

void signal_handler(int s) {
    printf("Caught signal %d\n",s);
    delete dataManager;
    exit(1);
}

int main(int argc, char* argv[]) {
    // Parse Cmd Line Args
    char * csvData = NULL;
    char * ipCamera = NULL;
    char * videoFile = NULL;
    char * csvBlobFile = NULL;
    char * csvObjectsDetectedFile = NULL;
    char * jpegDumpPath = NULL;

    // Media dimensions
    int w=0, h=0;

    // Optional cropping values
    int x=0, y=0, l=0, t=0;

    int c;
    int dataSources = 0;
    bool displayFrames = false;
    int fps = 0;
// TODO: add fps parameter,
    while ((c = getopt (argc, argv, "i:o:c:v:m:f:l:dj:x:y:l:t:w:h:?")) != -1) {
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
            case 'b':
                csvBlobFile = optarg;
                break;
            case 'o':
                csvObjectsDetectedFile = optarg;
                break;
            case 'd':
                displayFrames = true;
                break;
            case 'j':
                jpegDumpPath = optarg;
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

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    dataManager = new DataSourceManager();

    DataSourceManager& manager = *dataManager;

    // Configure Manager
    if (csvBlobFile) {
        manager.setCsvBlobFile(csvBlobFile);
    }

    if (csvObjectsDetectedFile) {
        manager.setCsvObjectsFile(csvObjectsDetectedFile);
    }

    if (l && t) {
        manager.getImageProcessor().setCrop(x, y, l, t);
    }

    if (jpegDumpPath) {
        manager.getImageProcessor().setJpegDumpPath(jpegDumpPath);
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
    delete dataManager;
}
