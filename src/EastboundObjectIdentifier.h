#ifndef _EASTBOUND_OBJECT_IDENTIFIER_H_
#define _EASTBOUND_OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include "ObjectIdentifier.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

class EastboundObjectIdentifier: public ObjectIdentifier {

public:
    EastboundObjectIdentifier(Blob* b);

    ~EastboundObjectIdentifier();

    //virtual int getFit(Blob& b);

    virtual long getTimeout() {
        return 10*1000;
    }

    virtual int getDirection() {
        return 1;
    }

    virtual ObjectIdentifier::ObjectType getType();

    static bool inStartingZone(Blob& b) {
        bool northLane = (b.x >= 0 && b.x <= 60 && b.y >= 75 && b.y <= 100);
        bool southLane = (b.x >= 45 && b.x <= 105 && b.y >= 115 && b.y <= 175);
        printf("EB Starting Zone N %d S %d NS %d\n", northLane, southLane, (northLane || southLane));
        return (northLane || southLane);
    }

    // Returns true if blob is in eastbound traffic lanes
    static bool isInRange(Blob& b) {
        double overlap = 0; // due to perspective, eastbound cars leak over into westbound direction
        double yCalc = 31 * b.x / 163 + (10442/163) - overlap;
        printf("YCalc %f Meas %f at x=%f\n", yCalc, b.y, b.x);
        return (b.y >= yCalc);
    }

private:

};

#endif

