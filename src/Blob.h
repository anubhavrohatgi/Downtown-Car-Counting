#ifndef _BLOB_H_
#define _BLOB_H_

#include <stdio.h>
static int blobsAlive = 0;
class Blob {

public:
    Blob(double x, double y, double area, long time) :
        x(x),
        y(y),
        area(area),
        time(time)
    {
        blobsAlive++;
        printf("Blobs Alive %d\n", blobsAlive);
    }

    void setClusterId(int id) {
        clusterId = id;
    }

    int getClusterId() {
        return clusterId;
    }

    ~Blob() {
        blobsAlive--;
        printf("Blobs Alive %d\n", blobsAlive);
    }

    double x;
    double y;
    double area;
    long time;
private:
    int clusterId;
};

#endif

