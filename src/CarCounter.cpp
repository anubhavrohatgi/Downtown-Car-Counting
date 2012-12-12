#include "CarCounter.h"

#include <limit>

CarCounter::CarCounter() :
    carCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0),
    frameNumber(0)
{

}

CarCounter::~CarCounter()
{
    printf("PRINTING ALL BLOBS ...\n");
    for (int i = 0; i < allBlobs.size(); i++) {
        unsigned int frameNum = allBlobs.at(i).label; // HACK, stored the frameNumber in the blob data
        printf("%d,%f,%f,%d,%d\n", frameNum, allBlobs.at(i).centroid.x, allBlobs.at(i).centroid.y, allBlobs.at(i).area, (int)allBlobs.at(i).p1);
    }
}

int CarCounter::updateStats(CvBlobs& blobs)
{
    updateStats(blobs, frameNumber++);
}

int CarCounter::updateStats(CvBlobs& blobs, int frameNum) {
    frameNumber = frameNum;
    int newROs = 0;
    int numBlobs = blobs.size();
    int numObjs = objects.size();

    // Classify the Blobs as Probable Road Objects
    for (CvBlobs::iterator it = blobs.begin(); it != blobs.end(); ++it) {

        CvBlob blob = *(it->second);
        blob.label = frameNum; // Hack to store the frame number in the blob data

        ObjectIdentifier * closestFit = NULL;

        double minDistanceFromPath = std::numeric_limits<double>::max();; //MAX_DOUBLE
        double minDistanceLastBlob = std::numeric_limits<double>::max();; //MAX_DOUBLE

        //printf("UpdateStats %d,%f,%f,%d\n", frameNum, blob.centroid.x, blob.centroid.y, blob.area);

        for (list<ObjectIdentifier>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            // Identifier we're testing
            ObjectIdentifier * oi = &*obj;
            bool inRange;
            double x = blob.centroid.x;
            if (blob.label == 463) {
                                printf("MMM\n");
                                //inRange = oi->inRange(blob);
                            }

            if (oi->inRange(blob)) {
                double distanceFromExpectedPath = oi->distFromExpectedPath(blob); // TODO: use?
                double distanceFromPathOfBlobs = oi->errFromLine(blob);
                double distanceToLastBlob = oi->distanceFromLastBlob(blob);

                bool inZone1 = ObjectIdentifier::inStartingZone(blob);
                bool inZone2 = ObjectIdentifier::inStartingZone(oi->getLastBlob());

                bool closeToExpectedPath = (distanceFromExpectedPath < 15);
                bool closeToPlottedPath = (distanceFromPathOfBlobs < 20);

                bool isClosestToPathOfBlobs = (distanceFromPathOfBlobs < minDistanceFromPath);
                if (isClosestToPathOfBlobs) {
                    minDistanceFromPath = distanceFromPathOfBlobs;
                }

                bool isClosestToLastBlob = (distanceToLastBlob < minDistanceLastBlob);
                if (isClosestToLastBlob) {
                    minDistanceLastBlob = distanceToLastBlob;
                }


                if ((closeToExpectedPath || closeToPlottedPath) && isClosestToLastBlob) { // TODO: use CONSTS
                    minDistanceFromPath = distanceFromPathOfBlobs;
                    closestFit = oi;
                } else if (distanceToLastBlob < 15) {
                    // If blob we're looking at is in the starting zone and identifier we're comparing with is also in starting zone, relax
                    // distanceFromExpectedPath criteria
                    closestFit = oi;
                }

            }
        }

        if (closestFit) {
            // Add to existing Object Identifier
            int id = closestFit->getId();
            blob.p1 = id;
            closestFit->addBlob(blob);
            //printf("ADD TO OBJ\n");
        } else if (ObjectIdentifier::inStartingZone(blob)) {
            // No suitable identifier exists, this may be a new road object, create new identifier
            ObjectIdentifier obj(blob);
            int id = obj.getId();
            blob.p1 = id;
            objects.push_back(obj);
            //printf("NEW OBJ\n");
        } else {
            // Not sure what this is
            blob.p1 = 0;
            unidentifiedBlobs.push_back(blob);
            //printf("UNIDENTIFIED BLOB\n");
        }
        allBlobs.push_back(blob);
    }

    // Iterate through ObjectIdentifiers and see if we can classify (or discard) them
    for (list<ObjectIdentifier>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

        ObjectIdentifier * oi = &*obj;
        oi->incrementFrameCount(); // Update 'age' counter

        int lastSeen = oi->getLastSeenNFramesAgo();
        double distanceTravelled = oi->distanceTravelled();
        int numBlobs = oi->getNumBlobs();

        if (lastSeen > 10 || (oi->lifetime() > 30 && oi->getSpeed() < 2)) { // TODO: Use CONSTS note: lifetime depends on FPS
            if (numBlobs >= 8) {
                if (oi->inEndZone()) {
                    carCount++;
                    obj = objects.erase(obj);
                    newROs++;
                    printf("END ZONE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().label, oi->getSpeed());
                    //oi->printPoints();
                } else if (oi->distanceTravelled() > 150) { // TODO: use consts, note: normal distance is about ~300
                    carCount++;
                    obj = objects.erase(obj);
                    newROs++;
                    printf("DISTANCE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().label, oi->getSpeed());
                    //oi->printPoints();
                } else {
                    // Discard identifier
                    printf("DISCARDED CAR numBlobs %d lastSeen %d dist %d speed %f\n", numBlobs, lastSeen, (int)distanceTravelled, oi->getSpeed());
                    obj->printPoints();
                    obj = objects.erase(obj);
                }
            } else {
                // Discard identifier
                printf("DISCARDED CAR numBlobs %d lastSeen %d dist %d speed %f\n", numBlobs, lastSeen, (int)distanceTravelled, oi->getSpeed());
                obj->printPoints();
                obj = objects.erase(obj);
            }
        }
    }
    return newROs;
}
