#ifndef _DATA_SOURCE_MANAGER_H_
#define _DATA_SOURCE_MANAGER_H_

#include "CarCounter.h"

class DataSourceManager {

public:
    DataSourceManager();
    ~DataSourceManager();

    void setCsvLogFile(const char * path);
    void setImgMask(const char * path);

    int processVideoFile(const char * path);

    // CSV File is expected with at least 4 columns
    // frameNumber, blob x, blob y, blob size
    int processCsvFile(const char * path);

    int processIpCamera(const char * ip);

private:
    CarCounter counter;

    bool logToCsv;
    bool useImgMask;
    const char * csvLogFile;
    const char * imgMaskFile;
};

#endif
