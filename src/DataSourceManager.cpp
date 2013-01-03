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
    printf("CSV Rows: %d\n", array.size());

    // Map of blobs
    // We store them per frame so that they can be processed per frame like they would be if generated from a video
    vector<Blob> blobs;
    int lastFrameNum = 0;

    for (int i = 0; i < array.size(); i++)
    {
        if (array[i].size() >= 4) { // Expect at least 4 columns
            // Read Blob from row
            int frameNum = atoi(array[i][0].c_str()) * 5;
            double x = atof(array[i][1].c_str());
            double y = atof(array[i][2].c_str());
            double area = atoi(array[i][3].c_str());
            Blob * b = new Blob(x, y, area, frameNum); // TODO: proper memory mgmt
            if (i == 0) {
                lastFrameNum = frameNum;
            }

            // New frame in the log file, process blobs stored for last frame
            if (frameNum != lastFrameNum) {
                counter.updateStats(blobs, blobs.at(0).frameNum);
                blobs.clear();
            }

            // Store new blob
            blobs.push_back(*b);
            lastFrameNum = frameNum;
        }
    }

    // Make sure blobs time out
    blobs.clear();
    for (int i = 0; i < 200; i++) { // TODO: get a number for this instead of 200
        counter.updateStats(blobs);
    }
    return counter.getCarCount();
}

int DataSourceManager::processIpCamera(const char * connectionString, int mediaWidth, int mediaHeight)
{
    if (!networkCamera) {
        networkCamera = new NetworkStream(connectionString, &getImageProcessor(), mediaWidth, mediaHeight);
    }
    networkCamera->startProcessing();
}
