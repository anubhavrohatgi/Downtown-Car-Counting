#ifndef _BLOB_H_
#define _BLOB_H_

#include <stdio.h>

class Blob {
public:
    Blob(double x, double y, double area, long time);
    Blob(double x, double y, double minx, double maxx, double miny, double maxy, double area, long time);

    void setClusterId(int id);

    int getClusterId();

    ~Blob();

    double x;
    double y;
    double area;
    double minx, maxx;
    double miny, maxy;
    long time;
private:
    int clusterId;
};

#endif

