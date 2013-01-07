#include "EastboundObjectIdentifier.h"

#include <limits>
#include <cmath>

using namespace std;
using namespace cv;

EastboundObjectIdentifier::EastboundObjectIdentifier(Blob b) :
        ObjectIdentifier(b)
{

}

bool EastboundObjectIdentifier::isInRange(Blob b)
{
    double yCalc = (31/163) * b.x + (10442/163);
    printf("YCalc %f\n", yCalc);
    return (b.y <= yCalc);
}

int EastboundObjectIdentifier::getFit(Blob b)
{
    return 0;
}

EastboundObjectIdentifier::~EastboundObjectIdentifier()
{
    //printf("~%d (#pts %d): (%.2f, %.2f, %.2f, %.2f) size %.2f\n", id, points.size(), minx, maxx, miny, maxy, size());
    // TODO: why can't I delete these ?
    //delete &xyFilter;
    //delete &txFilter;
    //delete &tyFilter;
    //delete &measurement;
}
