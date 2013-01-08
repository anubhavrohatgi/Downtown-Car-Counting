#include "EastboundObjectIdentifier.h"

#include <limits>
#include <cmath>

using namespace std;
using namespace cv;

EastboundObjectIdentifier::EastboundObjectIdentifier(Blob* b) :
        ObjectIdentifier(b)
{

}
#if 0
int EastboundObjectIdentifier::getFit(Blob& b)
{
    int score = 0;

    double distToLast = distanceFromLastBlob(b);
    double toPredictedXY = distToPredictedXY(b.x, b.y);
    double toPredictedTX = distToPredictedTX(b.time, b.x);
    double toPredictedTY = distToPredictedTY(b.time, b.y);
    double e1 = errXY(b.x, b.y);
    double e2 = errTX(b.time, b.x);
    double e3 = errTY(b.time, b.y);
    double e = sqrt(e1*e1 + e2*e2 + e3*e3);

    printf("Predicted XY %f TX %f TY %f\n", toPredictedXY, toPredictedTX, toPredictedTY);
    printf("ID %d XY %f TX %f TY %f   D %f      SumSq: %f\n", getId(), e1, e2, e3, distToLast, e);

    if (getNumBlobs() <= 5) {
        score = (30 - distToLast) * 3;
#if 0
    } else if (e < 20 && e < minError) {
        minError = e;
        bestFit = oi;
        printf("Best fit accepted\n");
#endif
    } else {
        // These 3 contribute 200 / 300 possible score
        int xyScore = 100 - (toPredictedXY < 100 ? toPredictedXY : 100);
        int txScore = 100 - (toPredictedTX < 100 ? toPredictedTX : 100);
        int tyScore = 100 - (toPredictedTY < 100 ? toPredictedTY : 100);
        int xytScore = xyScore + txScore + tyScore - 100;
        if (xytScore < 0) xytScore = 0;
        // Contributes up to 120
        int linearityScore = (e <= 60) ? (60 - e) * 2 : 0;
        score = xytScore + linearityScore;
        if (linearityScore && distToLast < 20) {
            score += (20 - distToLast) * 5;
        }
    }
    printf("Score %d\n", score);
    return score;
}
#endif
ObjectIdentifier::ObjectType EastboundObjectIdentifier::getType()
{
    // TODO: update parameters, add slope
    if (getAverageSize() > 400 && getNumBlobs() > 10 && getDistanceTravelled() > 200) {
        return VEHICLE;
    } else {
        return UNKNOWN;
    }
}

EastboundObjectIdentifier::~EastboundObjectIdentifier()
{
    printf("~ EastboundObjectIdentifier ID %d type %d size %f numBlobs %d dist %f lifetime %ld slope %f\n", getId(), getType(), getAverageSize(), getNumBlobs(), getDistanceTravelled(), getLifetime(), getXYSlope());
    //printf("~%d (#pts %d): (%.2f, %.2f, %.2f, %.2f) size %.2f\n", id, points.size(), minx, maxx, miny, maxy, size());
    // TODO: why can't I delete these ?
    //delete &xyFilter;
    //delete &txFilter;
    //delete &tyFilter;
    //delete &measurement;
}
