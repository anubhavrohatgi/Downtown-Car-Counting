#ifndef _BLOB_H_
#define _BLOB_H_

class Blob {

public:
    Blob() { }

    Blob(double x, double y, double area, long time) :
        x(x),
        y(y),
        area(area),
        time(time)
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
    long time;
private:
    int clusterId;
};

#endif

