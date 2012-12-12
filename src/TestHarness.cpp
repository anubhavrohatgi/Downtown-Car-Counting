#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "RoadObject.h"
#include "CarCounter.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/flann/flann_base.hpp"

/* time example */
#include <stdio.h>
#include <time.h>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>

#include <stdio.h>

#include "CarCounter.h"

using namespace cv;

using namespace std;
using namespace cvb;

int readCsv();

int m33ain(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage: %s PATH_TO_RAW_AVI_VIDEO_FILE [IMAGE_MASK_JPG_OR_OTHER_FORMAT]", argv[0]);
        //return 0;
    }
    // TODO: use string?
    //char * video_filename = argv[1];

    // READ CSV
    readCsv();

}

// From http://forums.codeguru.com/showthread.php?396459-Reading-CSV-file-into-an-array
int readCsv()
{
    using namespace std;

    //ifstream in("/Users/j3bennet/bah2.csv");
    ifstream in("/Users/j3bennet/r430to525.csv");
    //ifstream in("/Users/j3bennet/r859to1050.csv");

    string line, field;

    vector< vector<string> > array;  // the 2D array
    vector<string> v;                // array of values for one line only

    while ( getline(in,line) )    // get next line in file
    {
        //printf("LINE\n");
        v.clear();
        stringstream ss(line);

        while (getline(ss,field,','))  // break line into comma delimitted fields
        {
            v.push_back(field);  // add each field to the 1D array
        }

        array.push_back(v);  // add the 1D array to the 2D array
    }

    // print out what was read in

    CarCounter counter;

    printf("ARRAY SIZE: %d\n", array.size());

    CvBlobs blobs;

    int lastFrameNum = -1;

    CvBlobs::iterator it;

    for (size_t i=0; i<array.size(); ++i)
    {
        //printf("I SIZE %d\n", array[i].size());
        if (array[i].size() >= 4) {
            // Read Blob from file
            struct CvBlob * b = new CvBlob; // TODO: memory leak?
            int frameNum = atoi(array[i][0].c_str());
            b->centroid.x = atof(array[i][1].c_str());
            b->centroid.y = atof(array[i][2].c_str());
            b->area = atoi(array[i][3].c_str());
            //printf("%d,%f,%f,%d\n", frameNum, b->centroid.x, b->centroid.y, b->area);

            if (i == 0) {
                lastFrameNum = frameNum;
            }

            // New frame in the log file, process blobs for last frame
            if (frameNum != lastFrameNum && blobs.size() != 0) {
                //printf("Processing %d blobs %d -> %d\n", blobs.size(), lastFrameNum, frameNum);
                counter.updateStats(blobs, lastFrameNum);
                blobs.clear();
                //it = blobs.begin();
            } else {
                //++it;
            }

            // Insert new blob
            blobs.insert(pair<CvLabel,CvBlob*>(i, b));
            lastFrameNum = frameNum;
        }

        for (size_t j=0; j<array[i].size(); ++j)
        {
            //cout << array[i][j] << "|"; // (separate fields by |)
        }
        //cout << "\n";
    }

    // Make sure blobs time out
    blobs.clear();
    for (int i = 0; i < 200; i++) {
        counter.updateStats(blobs);
    }

    return 0;
}

