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
