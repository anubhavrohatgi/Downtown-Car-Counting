#ifndef _EASTBOUND_OBJECT_IDENTIFIER_H_
#define _EASTBOUND_OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include "ObjectIdentifier.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

class EastboundObjectIdentifier: public ObjectIdentifier {

public:
    EastboundObjectIdentifier(Blob& b);

    ~EastboundObjectIdentifier();

    // Returns fitness on scale of 0 - 100
    virtual int getFit(Blob& b);

    virtual long getTimeout() {
        return 10*1000;
    }

    virtual ObjectIdentifier::ObjectType getType();

    static bool inStartingZone(Blob& b) {
        return (b.x >= 0 && b.x <= 50 && b.y >= 75 && b.y <= 100);
    }

    // Returns true if blob is in eastbound traffic lanes
    static bool isInRange(Blob& b) {
        double yCalc = 31 * b.x / 163 + (10442/163);
        printf("YCalc %f Meas %f at x=%f\n", yCalc, b.y, b.x);
        return (b.y >= yCalc);
    }

private:

};

#endif

