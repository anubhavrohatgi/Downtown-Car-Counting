#ifndef _BLOB_H_
#define _BLOB_H_

class Blob {

public:
    Blob() { }

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
    int clusterId;
};

#endif

