#include "Blob.h"

Blob::Blob(double x, double y, double area, long time) :
    x(x),
    y(y),
    area(area),
    minx(0),
    maxx(0),
    miny(0),
    maxy(0),
    time(time)
{

}

Blob::Blob(double x, double y, double minx, double maxx, double miny, double maxy, double area, long time) :
    x(x),
    y(y),
    area(area),
    minx(minx),
    maxx(maxx),
    miny(miny),
    maxy(maxy),
    time(time)
{

}

void Blob::setClusterId(int id)
{
    clusterId = id;
}

int Blob::getClusterId()
{
    return clusterId;
}

Blob::~Blob()
{

}
