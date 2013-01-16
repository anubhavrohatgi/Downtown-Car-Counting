#include "Blob.h"

Blob::Blob(double x, double y, double area, long time) :
    x(x),
    y(y),
    area(area),
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
