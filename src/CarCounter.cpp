#include "CarCounter.h"

using namespace std;

CarCounter::CarCounter() :
    carCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0),
    logFile(NULL),
    logFilePath(NULL),
    writesToLog(0)
{

}

CarCounter::~CarCounter()
{
    // Log all remaining blobs
    classifyObjects(true);
    blobsToLogAndRemove(allBlobs.size());
    printf("EBO Size %d\n", eastboundObjects.size());

    printf("WBO Size %d\n", westboundObjects.size());
for (int i = 0; i < unidentifiedBlobs.size(); i++) {
        //delete unidentifiedBlobs.at(i);
    }
}

int CarCounter::updateStats(vector<Blob*>& blobs, long currentTime) {
    int numBlobs = blobs.size();
    if (numBlobs > 0) printf("Time %ld\n", blobs.front()->time);

    // Determine the best fit for each blob
    for (unsigned int i = 0; i < blobs.size(); i++) {
        Blob& blob = *blobs.at(i);
        ObjectIdentifier * bestFit = NULL;
        if(EastboundObjectIdentifier::isInRange(blob)) {
            bestFit = findBestFit(blob, eastboundObjects, currentTime);
        } else if (WestboundObjectIdentifier::isInRange(blob)) {
            bestFit = findBestFit(blob, westboundObjects, currentTime);
        }

        if (bestFit) {
            // Add to existing Object Identifier
            int id = bestFit->getId();
            blob.setClusterId(id);
            bestFit->addBlob(blob);
            printf("ADD TO OBJ %d\n", id);
        } else if (EastboundObjectIdentifier::inStartingZone(blob)) {
            // No suitable identifier exists, this may be a new object, create new identifier
            EastboundObjectIdentifier * obj = new EastboundObjectIdentifier(&blob); // TODO: use new and do memory mgmt
            int id = obj->getId();
            blob.setClusterId(id);
            eastboundObjects.push_back(obj);
            printf("NEW EAST OBJ %d\n", id);
        } else if (WestboundObjectIdentifier::inStartingZone(blob)) {
            // No suitable identifier exists, this may be a new object, create new identifier
            WestboundObjectIdentifier * obj = new WestboundObjectIdentifier(&blob); // TODO: use new and do memory mgmt
            int id = obj->getId();
            blob.setClusterId(id);
            westboundObjects.push_back(obj);
            printf("NEW WEST OBJ %d\n", id);
        } else {
            blob.setClusterId(1); // 1 = UNKNOWN
            unidentifiedBlobs.push_back(&blob);
            printf("UNIDENTIFIED BLOB\n");
        }
        allBlobs.push_back(&blob);
    }
    return classifyObjects(false);;
}

ObjectIdentifier* CarCounter::findBestFit(Blob& b, list<ObjectIdentifier*> objects, long currentTime)
{
    ObjectIdentifier* bestFit = NULL;
    int bestFitRecorded = 0;

    for (list<ObjectIdentifier*>::const_iterator iterator = objects.begin(), end = objects.end(); iterator != end; ++iterator) {
        ObjectIdentifier* oi = *iterator;
        int fit = oi->getFit(b);
        if (fit > bestFitRecorded) {
            bestFit = oi;
            bestFitRecorded = fit;
        }
    }
    return bestFit;
}

int CarCounter::classifyObjects(bool forceTimeout)
{
    int newlyClassified = 0;
    // Iterate through ObjectIdentifiers and see if we can classify (or discard) them
    std::list<ObjectIdentifier*>::iterator iterator = eastboundObjects.begin();
    while (iterator != eastboundObjects.end()) {
        ObjectIdentifier * oi = *iterator;
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout()) {
            // Object has timed out
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("EASTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
            }
            delete oi;
            eastboundObjects.erase(iterator++);
        } else {
            if (forceTimeout) assert(false);
            iterator++;
        }
    }

    iterator = westboundObjects.begin();
    while (iterator != westboundObjects.end()) {
        ObjectIdentifier * oi = *iterator;
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout()) {
            // Object has timed out
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("WESTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
            }
            delete oi;
            westboundObjects.erase(iterator++);
        } else {
            if (forceTimeout) assert(false);
            iterator++;
        }
    }

    // Log old blobs to file
    // TODO: better way to do memory mgmt
    if (allBlobs.size() > 500) {
        //blobsToLogAndRemove(300);
    }
    return newlyClassified;
}

void CarCounter::blobsToLogAndRemove(int numBlobs)
{
    if (allBlobs.size() < numBlobs) {
        numBlobs = allBlobs.size();
    }

    if (numBlobs == 0) return;
    printf("NUM BLOBS %d\n", numBlobs);
    char buf[120];

    if (writesToLog == 0) {
        // Create header and legend
        writeToLog("time,x,y,area,id\n"); // TODO: beef up logging
        for (int j = 1; j < 8; j++) {
            Blob& b = *allBlobs.front();
            double x = b.x + 35 * j;
            sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, x, b.y, 4000, j);
            writeToLog(buf);
        }
    }

    for (int i = 0; i < allBlobs.size(); i++) {
        Blob& b = *allBlobs.at(i);
        sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, b.x, b.y, (int)b.area, b.getClusterId());
        writeToLog(buf);
        delete &b;
    }
}

int CarCounter::writeToLog(const char * line)
{
    if (!logFilePath) return -1;

    if (writesToLog == 0) {
        logFile = fopen(logFilePath, "w");
    } else {
        logFile = fopen(logFilePath, "a");
    }
    fprintf(logFile, "%s", line);
    fclose(logFile);
    writesToLog++;
    return 0;
}
