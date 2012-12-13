#ifndef _BLOB_H_
#define _BLOB_H_

#include "cvblob.h"

using namespace cvb;

class Blob {

public:
    Blob() { }

    Blob(CvBlob b, int frameNumber) :
        blob(b),
        x(b.centroid.x),
        y(b.centroid.y),
        area(b.area),
        frameNum(frameNumber)
    {

    }

    Blob(double x, double y, double area, int frameNumber) :
        x(x),
        y(y),
        area(area),
        frameNum(frameNumber)
    {

    }

    Blob(double x, double y, int frameNumber) :
        x(x),
        y(y),
        frameNum(frameNumber)
    {

    }

    void setClusterId(int id) {
        clusterId = id;
    }

    int getClusterId() {
        return clusterId;
    }

    ~Blob() {

    }

    double x;
    double y;
    double area;
    int frameNum;

private:
    CvBlob blob;
    int clusterId;
};

#endif

