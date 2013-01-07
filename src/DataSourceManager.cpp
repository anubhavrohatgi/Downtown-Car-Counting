#include "DataSourceManager.h"
#include <fstream>

using namespace std;

DataSourceManager::DataSourceManager() :
    imgProcessor(NULL),
    networkCamera(NULL)
{

}

DataSourceManager::~DataSourceManager()
{
    delete imgProcessor;
    delete networkCamera;
}

void DataSourceManager::setCsvLogFile(const char * path)
{
    counter.setOutputLogFile(path);
}

int DataSourceManager::processVideoFile(const char * path)
{
    getImageProcessor().processVideoFile(path);
}

int DataSourceManager::processCsvFile(const char * path)
{
    // CSV Reading Code from: http://forums.codeguru.com/showthread.php?396459-Reading-CSV-file-into-an-array
    std::ifstream in(path);

    string line, field;
    vector< vector<string> > array;  // the 2D array
    vector<string> v;                // array of values for one line only

    while (getline(in,line)) {    // get next line in file
        v.clear();
        stringstream ss(line);
        while (getline(ss,field,',')) { // break line into comma delimitted fields
            v.push_back(field);  // add each field to the 1D array
        }
        array.push_back(v);  // add the 1D array to the 2D array
    }

    // SUGGEST: Could process lines as they're read in if dealing with huge data
    printf("CSV Rows: %d\n", (int)array.size());

    // We store them per frame so that they can be processed per frame like they would be if generated from a video
    vector<Blob*> blobs;
    long lastTime = -1;

    for (int i = 0; i < array.size(); i++)
    {
        if (array[i].size() >= 4) { // Expect at least 4 columns
            // Read Blob from row
            long time = atoi(array[i][0].c_str());
            double x = atof(array[i][1].c_str());
            double y = atof(array[i][2].c_str());
            double area = atoi(array[i][3].c_str());

            if (area != 4000 && x > 0 && y > 0) { // Hack, using area = 4000 as a legend when graphing in R
                Blob* b = new Blob(x, y, area, time);
                if (lastTime == -1) {
                    lastTime = time;
                }

                // New frame in the log file, process blobs stored for last frame
                if (time != lastTime && blobs.size() > 0) {
                    counter.updateStats(blobs, lastTime);
                    blobs.clear();
                }

                // Store new blob
                blobs.push_back(b);
                lastTime = time;
            }
        }
    }

#if 0 // TODO: shouldn't be necessary
    // Make sure blobs time out
    blobs.clear();
    for (int i = 0; i < 200; i++) { // TODO: get a number for this instead of 200
        counter.updateStats(blobs);
    }
#endif
    return counter.getCarCount();
}

int DataSourceManager::processIpCamera(const char * connectionString, int mediaWidth, int mediaHeight)
{
    if (!networkCamera) {
        networkCamera = new NetworkStream(connectionString, &getImageProcessor(), mediaWidth, mediaHeight);
    }
    networkCamera->startProcessing();
}
