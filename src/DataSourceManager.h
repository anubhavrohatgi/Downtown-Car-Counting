#ifndef _DATA_SOURCE_MANAGER_H_
#define _DATA_SOURCE_MANAGER_H_

#include "CarCounter.h"
#include "ImageProcessor.h"
#include "NetworkStream.h"

class DataSourceManager {

public:
    DataSourceManager();
    ~DataSourceManager();

    void setCsvLogFile(const char * path);

    int processVideoFile(const char * path);

    ImageProcessor& getImageProcessor() {
        if (!imgProcessor) {
            imgProcessor = new  ImageProcessor(&counter);
        }
        return *imgProcessor;
    }

    // CSV File is expected with at least 4 columns
    // frameNumber or timestamp, blob x, blob y, blob size
    int processCsvFile(const char * path);

    int processIpCamera(const char * connectionString, int mediaWidth, int mediaHeight);

private:
    CarCounter counter;

    ImageProcessor * imgProcessor;
    NetworkStream  * networkCamera;
};

#endif
