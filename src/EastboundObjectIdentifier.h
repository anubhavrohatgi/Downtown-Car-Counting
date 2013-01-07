#ifndef _EASTBOUND_OBJECT_IDENTIFIER_H_
#define _EASTBOUND_OBJECT_IDENTIFIER_H_

#include "Blob.h"
#include "ObjectIdentifier.h"
#include <vector>
#include <opencv2/video/tracking.hpp>

class EastboundObjectIdentifier: public ObjectIdentifier {

public:
    EastboundObjectIdentifier(Blob b);

    ~EastboundObjectIdentifier();

    // Returns fitness on scale of 0 - 100
    virtual int getFit(Blob b);

    // Returns true if blob is in eastbound traffic lanes
    static bool isInRange(Blob b);

private:

};

#endif

