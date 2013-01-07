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
    // Shutting down, classify remaining objects
    classifyObjects(true, 0);
    assert(eastboundObjects.size() == 0);
    assert(westboundObjects.size() == 0);
    for (int i = 0; i < unidentifiedBlobs.size(); i++) {
        delete unidentifiedBlobs.at(i);
    }
}

int CarCounter::processBlobs(vector<Blob*>& blobs, long currentTime) {
    int numBlobs = blobs.size();
    if (numBlobs > 0) printf("Time %ld\n", blobs.front()->time);
    printf("Updating Stats numBlobs %d  time %ld\n", (int)blobs.size(), currentTime);
    // Determine the best fit for each blob
    for (unsigned int i = 0; i < blobs.size(); i++) {
        Blob& blob = *blobs.at(i);
        ObjectIdentifier * bestFit = NULL;
        if(EastboundObjectIdentifier::isInRange(blob)) {
            bestFit = findBestFit(blob, eastboundObjects);
        } else if (WestboundObjectIdentifier::isInRange(blob)) {
            bestFit = findBestFit(blob, westboundObjects);
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
        logBlob(blob);
    }
    return classifyObjects(false, currentTime);
}

ObjectIdentifier* CarCounter::findBestFit(Blob& b, list<ObjectIdentifier*> objects)
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

int CarCounter::classifyObjects(bool forceTimeout, long currentTime)
{
    int newlyClassified = 0;
    // Iterate through ObjectIdentifiers and see if we can classify (or discard) them
    std::list<ObjectIdentifier*>::iterator iterator = eastboundObjects.begin();
    while (iterator != eastboundObjects.end()) {
        ObjectIdentifier * oi = *iterator;
        if (currentTime) {
            oi->updateTime(currentTime);
        }
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout()) {
            // Object has timed out
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("EASTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
            }
            std::vector<Blob*>& blobs = oi->getBlobs();
            for (int i = 0; i < blobs.size(); i++) {
                logBlob(*blobs.at(i));
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
        if (currentTime) {
            oi->updateTime(currentTime);
        }
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout()) {
            // Object has timed out
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("WESTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
            }
            std::vector<Blob*>& blobs = oi->getBlobs();
            for (int i = 0; i < blobs.size(); i++) {
                logBlob(*blobs.at(i));
            }
            delete oi;
            westboundObjects.erase(iterator++);
        } else {
            if (forceTimeout) assert(false);
            iterator++;
        }
    }
    return newlyClassified;
}

void CarCounter::logBlob(Blob& b)
{
    // TODO: store frameNum and blob dimensions?
    char buf[120];
    if (writesToLog == 0) {
        // Create header and legend
        writeToLog("time,x,y,area,id\n"); // TODO: beef up logging
        for (int j = 1; j < 8; j++) {
            double x = b.x + 35 * j;
            sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, x, b.y, 4000, j);
            writeToLog(buf);
        }
    }
    sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, b.x, b.y, (int)b.area, b.getClusterId());
    writeToLog(buf);
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
