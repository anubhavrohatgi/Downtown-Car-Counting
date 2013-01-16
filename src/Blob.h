#ifndef _BLOB_H_
#define _BLOB_H_

#include <stdio.h>

class Blob {
public:
    Blob(double x, double y, double area, long time);

    void setClusterId(int id);

    int getClusterId();

    ~Blob();

    double x;
    double y;
    double area;
    long time;
private:
    int clusterId;
};

#endif

