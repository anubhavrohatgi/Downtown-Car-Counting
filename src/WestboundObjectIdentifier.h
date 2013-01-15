#ifndef _WESTBOUND_OBJECT_IDENTIFIER_H_
#define _WESTBOUND_OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include "ObjectIdentifier.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

class WestboundObjectIdentifier: public ObjectIdentifier {

public:
    WestboundObjectIdentifier(Blob* b);

    ~WestboundObjectIdentifier();

    //virtual int getFit(Blob& b);

    virtual long getTimeout() {
        return 60*1000;
    }

    virtual int getDirection() {
        return 0;
    }

    virtual ObjectIdentifier::ObjectType getType();

    static bool inStartingZone(Blob& b) {
        return (b.x >= 290 && b.x <= 340 && b.y >= 75 && b.y <= 100);
    }

    // Returns true if blob is in Westbound traffic lanes
    static bool isInRange(Blob& b) {
        double yCalc = 31 * b.x / 163 + (10442/163);
        return (b.y < yCalc);
    }

private:

};

#endif

