#include "CarCounter.h"

using namespace std;

CarCounter::CarCounter() :
    eastboundCarCount(0),
    westboundCarCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0),
    blobLogFile(NULL),
    blobLogFilePath(NULL),
    objectDetectedFilePath(NULL),
    writesToBlobLog(0),
    writesToObjectLog(0)
{

}

CarCounter::~CarCounter()
{
    // Shutting down, classify remaining objects
    printf("~CarCounter\n");
    classifyObjects(true, 0);
    assert(eastboundObjects.size() == 0);
    assert(westboundObjects.size() == 0);
    for (int i = 0; i < unidentifiedBlobs.size(); i++) {
        delete unidentifiedBlobs.at(i);
    }
    printf("Eastbound %d Cars\n", eastboundCarCount);
    printf("Westbound %d Cars\n", westboundCarCount);
}

int CarCounter::processBlobs(vector<Blob*>& blobs, long currentTime, bool retryUnclassified) {
    // TODO: this is a hack, put it somewhere else
    for (int i = 0; i < unidentifiedBlobs.size(); i++) {
        delete unidentifiedBlobs.at(i);
    }
    unidentifiedBlobs.clear();
    int numBlobs = blobs.size();
    // Determine the best fit for each blob
    for (unsigned int i = 0; i < blobs.size(); i++) {
        Blob& blob = *blobs.at(i);
        int bestFitEB=0, bestFitWB=0;
        printf("\ntime=%ld  x=%f  y=%f  numBlobs %d\n", currentTime, blob.x, blob.y, (int)blobs.size());
        ObjectIdentifier* bestFit = NULL;
        ObjectIdentifier* pBestFitEB = NULL;
        ObjectIdentifier* pBestFitWB = NULL;
        if(EastboundObjectIdentifier::isInRange(blob)) {
            bestFitEB = findBestFit(blob, eastboundObjects, &pBestFitEB);
        }

        // Since there's some overlap between EB and WB, may need to test blob against EB and WB identifiers
        if (WestboundObjectIdentifier::isInRange(blob)) {
            bestFitWB = findBestFit(blob, westboundObjects, &pBestFitWB);
        }

        if (bestFitEB > bestFitWB) {
            bestFit = pBestFitEB;
        } else {
            bestFit = pBestFitWB;
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
            printf("UNIDENTIFIED BLOB\n");
            blob.setClusterId(1); // 1 = UNKNOWN
            unidentifiedBlobs.push_back(&blob);
        }
        logBlob(blob);
    }
    if (false) {
        printf("Processing Unknown Blobs...\n");
        processBlobs(unidentifiedBlobs, currentTime, false);
        unidentifiedBlobs.clear();
        printf("Done Processing Unknown Blobs\n");
    }
    // Run through unidentified blobs, see if we get any new matches
    return classifyObjects(false, currentTime);
}

int CarCounter::findBestFit(Blob& b, list<ObjectIdentifier*> objects, ObjectIdentifier** bestFit)
{
    ObjectIdentifier* bfit = NULL;
    int bestFitRecorded = 0;

    for (list<ObjectIdentifier*>::const_iterator iterator = objects.begin(), end = objects.end(); iterator != end; ++iterator) {
        ObjectIdentifier* oi = *iterator;
        int fit = oi->getFit(b);
        if (fit > bestFitRecorded) {
            bfit = oi;
            bestFitRecorded = fit;
        }
    }
    *bestFit = bfit;
    return bestFitRecorded;
}

int CarCounter::classifyObjects(bool forceTimeout, long currentTime)
{
    int newlyClassified = 0;
    int blobsOutstanding = 0;
    // Iterate through ObjectIdentifiers and see if we can classify (or discard) them
    std::list<ObjectIdentifier*>::iterator iterator = eastboundObjects.begin();
    while (iterator != eastboundObjects.end()) {
        ObjectIdentifier * oi = *iterator;
        printf("EB %d: %d blobs %ld lifetime\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime());
        blobsOutstanding += oi->getNumBlobs();
        if (currentTime) {
            oi->updateTime(currentTime);
        }
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout() || oi->getLifetime() > 2*60*1000) {
            // Object has timed out
            if (oi->getLifetime() > 2*60*1000) {
                printf("LIFETIME TIMEOUT\n");
            }
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("EASTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
                eastboundCarCount++;
            }
#if 0
            std::vector<Blob*>& blobs = oi->getBlobs();
            for (int i = 0; i < blobs.size(); i++) {
                logBlob(*blobs.at(i));
            }
#endif
            logIdentifiedObject(*oi, currentTime);
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
        printf("WB %d: %d blobs %ld lifetime\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime());
        blobsOutstanding += oi->getNumBlobs();
        if (currentTime) {
            oi->updateTime(currentTime);
        }
        long lastSeen = forceTimeout ? numeric_limits<long>::max() : oi->lastSeen();
        if (lastSeen > oi->getTimeout() || oi->getLifetime() > 2*60*1000) {
            // Object has timed out
            if (oi->getLifetime() > 2*60*1000) {
                printf("LIFETIME TIMEOUT\n");
            }
            if (oi->getType() != ObjectIdentifier::UNKNOWN) {
                printf("WESTBOUND OBJECT IDENTIFIED - type %d\n", oi->getType());
                printf("ID %d, %d pts age %ld dist %f\n", oi->getId(), oi->getNumBlobs(), oi->getLifetime(), oi->getDistanceTravelled());
                newlyClassified++;
                westboundCarCount++;
            }
#if 0
            std::vector<Blob*>& blobs = oi->getBlobs();
            for (int i = 0; i < blobs.size(); i++) {
                logBlob(*blobs.at(i));
            }
#endif
            logIdentifiedObject(*oi, currentTime);
            delete oi;
            westboundObjects.erase(iterator++);
        } else {
            if (forceTimeout) assert(false);
            iterator++;
        }
    }
    printf("Blobs Outstanding %d\n", blobsOutstanding);
    return newlyClassified;
}

void CarCounter::logBlob(Blob& b)
{
    // TODO: store frameNum, human-readable time, and blob dimensions?
    char buf[120];
    if (writesToBlobLog == 0) {
        // Create header and legend
        writeToBlobLog("time,x,y,area,id\n"); // TODO: beef up logging
        for (int j = 1; j < 8; j++) {
            double x = b.x + 35 * j;
            sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, x, b.y, 4000, j);
            writeToBlobLog(buf);
        }
    }
    if (b.getClusterId() != 1 || true) {
        sprintf(buf, "%ld,%f,%f,%d,%d\n", b.time, b.x, b.y, (int)b.area, b.getClusterId());
        writeToBlobLog(buf);
    }
}

void CarCounter::logIdentifiedObject(ObjectIdentifier& oi, long time)
{
    // TODO: store frameNum, human-readable time, and blob dimensions?
    char buf[120];
    if (writesToObjectLog == 0) {
        // Create a legend
        writeToObjectsLog("time,type,direction(W=0 E=1),distance,numBlobs,lifetime(ms),custerID\n"); // TODO: beef up logging
    }
    sprintf(buf, "%ld,%d,%c,%f,%d,%ld,%d\n", time, oi.getType(), oi.getDirection() == 0 ? 'W' : 'E', oi.getDistanceTravelled(), oi.getNumBlobs(), oi.getLifetime(), oi.getId());
    writeToObjectsLog(buf);
}

int CarCounter::writeToObjectsLog(const char * line)
{
    if (!objectDetectedFilePath) return -1;

    objectsLogFile = fopen(objectDetectedFilePath, "a");
    fprintf(objectsLogFile, "%s", line);
    fclose(objectsLogFile);
    writesToObjectLog++;
    return 0;
}

int CarCounter::writeToBlobLog(const char * line)
{
    if (!blobLogFilePath) return -1;

    if (writesToBlobLog == 0) {
        blobLogFile = fopen(blobLogFilePath, "w");
    } else {
        blobLogFile = fopen(blobLogFilePath, "a");
    }
    fprintf(blobLogFile, "%s", line);
    fclose(blobLogFile);
    writesToBlobLog++;
    return 0;
}
