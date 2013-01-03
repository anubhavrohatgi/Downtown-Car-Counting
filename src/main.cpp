#include <sys/time.h>

#include "DataSourceManager.h"
#include "ImageProcessor.h"

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
// TODO: add fps parameter, w, h params
    while ((c = getopt (argc, argv, "i:o:c:v:m:f:l:dx:y:l:t:w:h:")) != -1) {
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
                break;
            case 'h':
                break;
        // Cropping
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case 'l':
                w = atoi(optarg);
                break;
            case 't':
                h = atoi(optarg);
                break;
        // Outputs
            case 'o':
                csvLogFile = optarg;
                break;
            case 'd':
                displayFrames = true;
                break;
            default:
                abort();
        }
    }

    // Filter out invalid combinations
    if (dataSources > 1) {
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
    } else if (w && h) {
        manager.getImageProcessor().setCrop(x, y, l, t);
    }

    // Process Data or Stream from Camera
    if (csvData) {
        manager.processCsvFile(csvData);
    } else {
        manager.getImageProcessor().setShowFrames(displayFrames);
        if (ipCamera) {
            manager.processIpCamera(ipCamera, w, h);
        } else if (videoFile) {
            manager.processVideoFile(videoFile);
        }
    }
}
