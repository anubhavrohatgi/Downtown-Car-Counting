#include "CarCounter.h"

using namespace std;

CarCounter::CarCounter() :
    carCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0),
    frameNumber(0),
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
}

int CarCounter::updateStats(vector<Blob>& blobs)
{
    updateStats(blobs, ++frameNumber);
}

int CarCounter::updateStats(vector<Blob>& blobs, int frameNum) {
    frameNumber = frameNum;
    int numBlobs = blobs.size();
    int numObjs = objects.size();
    if (numBlobs > 0) printf("Frame %d\n", blobs.front().frameNum);
    // Classify the Blobs as Probable Road Objects
    for (int i = 0; i < blobs.size(); i++) {

        Blob blob = blobs.at(i);
        blob.frameNum = frameNumber; // Hack to store the frame number in the blob data

        ObjectIdentifier * bestFit = NULL;
        ObjectIdentifier * candidateFit = NULL; // Used if an identified blob is on the edge of our parameters

        double minError = std::numeric_limits<double>::max(); //MAX_DOUBLE

        printf("\nBlob for Frame %d, (x,y) %f,%f\n", frameNum, blob.x, blob.y);
        if (ObjectIdentifier::inStartingZone(blob)) {
            printf("In starting zone\n");
        } else {
            printf("Not in starting zone\n");
        }
        for (list<ObjectIdentifier>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            // Identifier we're testing
            ObjectIdentifier * oi = &*obj;
            int oiID = oi->getId();
#if 1
            double distToLast = oi->distanceFromLastBlob(blob);
            double toPredictedXY = oi->distToPredictedXY(blob.x, blob.y);
            double toPredictedTX = oi->distToPredictedTX(frameNum, blob.x);
            double toPredictedTY = oi->distToPredictedTY(frameNum, blob.y);
            double e1 = oi->errXY(blob.x, blob.y);
            double e2 = oi->errTX(blob.frameNum, blob.x);
            double e3 = oi->errTY(blob.frameNum, blob.y);
            double e = sqrt(e1*e1 + e2*e2 + e3*e3);

            printf("Predicted XY %f TX %f TY %f\n", toPredictedXY, toPredictedTX, toPredictedTY);
            printf("ID %d XY %f TX %f TY %f   D %f      SumSq: %f\n", oiID, e1, e2, e3, oi->distanceFromLastBlob(blob), e);

            if (oi->getNumBlobs() <= 5) {
                if (distToLast < 30 && distToLast < minError) {
                    minError = distToLast;
                    bestFit = oi;
                    printf("Best fit accepted\n");
                }
            } else if (e < 20 && e < minError) {
                minError = e;
                bestFit = oi;
                printf("Best fit accepted\n");
            } else if (toPredictedXY < 30 && toPredictedXY < minError) {
                minError = toPredictedXY;
                bestFit = oi;
                printf("Best fit accepted\n");
            }

#else
            double x = blob.x;
            if (oi->getNumBlobs() >= 2) {
                double err1 = oi->distFromExpectedX(blob.x, blob.frameNum);
                double err2 = oi->distFromExpectedY(blob.x, blob.y);
                double err3 = oi->distFromExpectedY(blob.y, (int)blob.frameNum);
                double err4 = oi->distToPredictedXY(blob.x, blob.y);
                printf("OI %d Err from Predicted %f\n", oi->getId(), err4);
                if (oi->inStartingZone(blob)) printf("IN STARTING ZONE\n");
                double errold = err1 + err2 + err3;
                double err = sqrt(err1 * err1 + err2 * err2 + err3 * err3);
                double localmin = std::min(std::min(err1, err2), std::min(err2, err3));

                double e1 = oi->errXY(blob.x, blob.y);
                double e2 = oi->errTX(blob.frameNum, blob.x);
                double e3 = oi->errTY(blob.frameNum, blob.y);
                double e = sqrt(e1*e1 + e2*e2 + e3*e3);
                double eold = e1 + e2 + e3;

                if (e < minError) { // TODO: tune
                    if (e < 50) {
                        if (!oi->inStartingZone(blob) || (oi->inStartingZone(blob) && oi->distanceFromLastBlob(blob) < 50)) {
                            minError = err;
                            bestFit = oi;
                        }
                    } else if (!oi->inStartingZone(blob) && e2 < 20) { // minimum of all 3 errors
                        //candidateFit = oi; // NOTE: this was a bad strategy.
                    }
                }
                //printf("ID %d ERRs (x given t) %f (y given x) %f (y given t) %f D %f SumSq: %f\n", oiID, err1, err2, err3, oi->distanceFromLastBlob(blob), err);
                printf("ID %d XY %f TX %f TY %f   D %f                                SumSq: %f\n", oiID, e1, e2, e3, oi->distanceFromLastBlob(blob), e);
                printf("ID %d txR  %f  tyR  %f  xyR  %f  Sum: %f\n", oiID, oi->txR, oi->tyR, oi->xyR, oi->txR + oi->tyR + oi->xyR);
                //inRange = oi->inRange(blob);
            } else if (minError == std::numeric_limits<double>::max() &&
                    oi->distanceFromLastBlob(blob) <= 15) {
                bestFit = oi;
            }
#endif
        }

        if (bestFit) {
            // Add to existing Object Identifier
            int id = bestFit->getId();
            blob.setClusterId(id);
            bestFit->addBlob(blob);
            printf("ADD TO OBJ %d\n", id);
        } else if (candidateFit) {
            int id = candidateFit->getId();
            blob.setClusterId(id);
            candidateFit->addBlob(blob);
            printf("ADD TO CANDIDATE OBJ %d\n", id);
        } else if (ObjectIdentifier::inStartingZone(blob)) {
            // No suitable identifier exists, this may be a new road object, create new identifier
            ObjectIdentifier obj(blob);
            int id = obj.getId();
            blob.setClusterId(id);
            objects.push_back(obj);
            printf("NEW OBJ %d\n", id);
        } else {
            // Not sure what this is
            blob.setClusterId(1);
            unidentifiedBlobs.push_back(blob);
            printf("UNIDENTIFIED BLOB\n");
        }
        allBlobs.push_back(blob);
    }

    classifyObjects(false);

    return 0;
}

int CarCounter::classifyObjects(bool forceAll)
{
    int newROs = 0;
    // Iterate through ObjectIdentifiers and see if we can classify (or discard) them
    for (list<ObjectIdentifier>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

        ObjectIdentifier * oi = &*obj;
        oi->incrementFrameCount(); // Update 'age' counter

        int lastSeen = forceAll ? 1000 : oi->getLastSeenNFramesAgo(); // if forceAll is set, set lastSeen artificially high for each object
        double distanceTravelled = oi->distanceTravelled();
        int numBlobs = oi->getNumBlobs();

        if (lastSeen > 10 || (oi->lifetime() > 30 && oi->getSpeed() < 2)) { // TODO: Use CONSTS note: lifetime depends on FPS
            if (numBlobs >= 8) {
                if (oi->inEndZone()) {
                    carCount++;
                    obj = objects.erase(obj);
                    newROs++;
                    printf("END ZONE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().frameNum, oi->getSpeed());
                    printf("ID %d, %d pts  txR  %f  tyR  %f  xyR  %f  Sum: %f\n", oi->getId(), oi->getNumBlobs(), oi->txR, oi->tyR, oi->xyR, oi->txR + oi->tyR + oi->xyR);
                    //oi->printPoints();
                } else if (oi->distanceTravelled() > 150) { // TODO: use consts, note: normal distance is about ~300
                    carCount++;
                    obj = objects.erase(obj);
                    newROs++;
                    printf("DISTANCE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().frameNum, oi->getSpeed());
                    printf("ID %d, %d pts  txR  %f  tyR  %f  xyR  %f  Sum: %f\n", oi->getId(), oi->getNumBlobs(), oi->txR, oi->tyR, oi->xyR, oi->txR + oi->tyR + oi->xyR);
                    //oi->printPoints();
                } else {
                    // Discard identifier
                    printf("DISCARDED CAR numBlobs %d lastSeen %d dist %d speed %f\n", numBlobs, lastSeen, (int)distanceTravelled, oi->getSpeed());
                    printf("ID %d, %d pts  txR  %f  tyR  %f  xyR  %f  Sum: %f\n", oi->getId(), oi->getNumBlobs(), oi->txR, oi->tyR, oi->xyR, oi->txR + oi->tyR + oi->xyR);
                    //obj->printPoints();
                    obj = objects.erase(obj);
                }
            } else {
                // Discard identifier
                printf("DISCARDED CAR numBlobs %d lastSeen %d dist %d speed %f\n", numBlobs, lastSeen, (int)distanceTravelled, oi->getSpeed());
                printf("ID %d, %d pts  txR  %f  tyR  %f  xyR  %f  Sum: %f\n", oi->getId(), oi->getNumBlobs(), oi->txR, oi->tyR, oi->xyR, oi->txR + oi->tyR + oi->xyR);
                //obj->printPoints();
                obj = objects.erase(obj);
            }
        }
    }

    // Log old blobs to file
    if (allBlobs.size() > 500) {
        blobsToLogAndRemove(300);
    }
    return newROs;
}

void CarCounter::blobsToLogAndRemove(int numBlobs)
{
    if (allBlobs.size() < numBlobs) {
        numBlobs = allBlobs.size();
    }

    if (numBlobs == 0) return;

    char buf[120];

    if (writesToLog == 0) {
        // Create header and legend
        writeToLog("time,x,y,area,id\n"); // TODO: beef up logging
        for (int j = 1; j < 8; j++) {
            Blob b = allBlobs.front();
            sprintf(buf, "%d,%f,%f,%d,%d\n", b.frameNum, b.x + 35 * j, b.y, 4000, j);
            writeToLog(buf);
        }
    }

    for (int i = 0; i < numBlobs; i++) {
        Blob b = allBlobs.front();
        allBlobs.pop_front();
        unsigned int frameNum = b.frameNum;
        sprintf(buf, "%d,%f,%f,%d,%d\n", frameNum, b.x, b.y, (int)b.area, b.getClusterId());
        writeToLog(buf);
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
