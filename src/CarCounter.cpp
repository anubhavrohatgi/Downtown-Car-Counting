#include "CarCounter.h"

#include <limit>

FILE *f = NULL;

int write_to_file_overwrite(char const *fileName, char * line)
{
    //printf("To File: %s", line);
    if (!f) {
        f = fopen(fileName, "w");
    }
    if (f == NULL) return -1;
    // you might want to check for out-of-disk-space here, too
    fprintf(f, "%s", line);
//    fclose(f);
    return 0;
}

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
    char buf[120];
    printf("PRINTING ALL BLOBS ...\n");
    for (int i = 0; i < allBlobs.size(); i++) {
        unsigned int frameNum = allBlobs.at(i).frameNum; // HACK, stored the frameNumber in the blob data
        if (i == 0) {
            for (int j = 1; j < 8; j++) {
                // Create legend
                sprintf(buf, "%d,%f,%f,%d,%d\n", frameNum, allBlobs.at(i).x + 35 * j, allBlobs.at(i).y, 4000, j);
                write_to_file_overwrite("/Users/j3bennet/dev.csv", buf);
                printf("%s", buf);
            }
        }
        sprintf(buf, "%d,%f,%f,%d,%d\n", frameNum, allBlobs.at(i).x, allBlobs.at(i).y, (int)allBlobs.at(i).area, allBlobs.at(i).getClusterId());
        write_to_file_overwrite("/Users/j3bennet/dev.csv",buf);
        printf("%s", buf);
    }
}

int CarCounter::updateStats(vector<Blob>& blobs)
{
    updateStats(blobs, frameNumber++);
}

int CarCounter::updateStats(vector<Blob>& blobs, int frameNum) {
    frameNumber = frameNum;
    int newROs = 0;
    int numBlobs = blobs.size();
    int numObjs = objects.size();

    // Classify the Blobs as Probable Road Objects
    for (int i = 0; i < blobs.size(); i++) {

        Blob blob = blobs.at(i);
        blob.frameNum = frameNum; // Hack to store the frame number in the blob data

        ObjectIdentifier * bestFit = NULL;
        ObjectIdentifier * candidateFit = NULL; // Used if an identified blob is on the edge of our parameters

        double minError = std::numeric_limits<double>::max(); //MAX_DOUBLE

        printf("Blob %d,%f,%f,%d\n", frameNum, blob.x, blob.y, blob.area);
        //printf("%d,%f,%f,%d,%d\n", frameNum, allBlobs.at(i).x + 35 * j, allBlobs.at(i).y, 4000, j);
        for (list<ObjectIdentifier>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            // Identifier we're testing
            ObjectIdentifier * oi = &*obj;
            int oiID = oi->getId();
            double x = blob.x;
            if (oi->getNumBlobs() >= 2) {
                double err1 = oi->distFromExpectedX(blob.x, blob.frameNum);
                double err2 = oi->distFromExpectedY(blob.x, blob.y);
                double err3 = oi->distFromExpectedY(blob.y, (int)blob.frameNum);
                double err = err1 + err2 + err3;
                double localmin = std::min(std::min(err1, err2), std::min(err2, err3));

                double e1 = oi->errXY(blob.x, blob.y);
                double e2 = oi->errTX(blob.frameNum, blob.x);
                double e3 = oi->errTY(blob.frameNum, blob.y);
                double e = e1 + e2 + e3;

                if (e < minError) { // TODO: tune
                    if (e < 27) {
                        minError = err;
                        bestFit = oi;
                    } else if (localmin < 10 && blob.x > 550) { // minimum of all 3 errors
                        //candidateFit = oi; // NOTE: this was a bad strategy.
                    }
                }
                printf("ID %d ERRs (x given t) %f (y given x) %f (y given t) %f D %f Sum: %f\n", oiID, err1, err2, err3, oi->distanceFromLastBlob(blob), err);
                printf("ID %d XY %f TX %f TY %f   D %f                                Sum: %f\n", oiID, e1, e2, e3, oi->distanceFromLastBlob(blob), e);
                //inRange = oi->inRange(blob);
            } else if (minError == std::numeric_limits<double>::max() &&
                    oi->distanceFromLastBlob(blob) <= 15) {
                bestFit = oi;
            }
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
            //printf("NEW OBJ\n");
        } else {
            // Not sure what this is
            blob.setClusterId(1);
            unidentifiedBlobs.push_back(blob);
            printf("UNIDENTIFIED BLOB\n");
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
                    printf("END ZONE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().frameNum, oi->getSpeed());
                    //oi->printPoints();
                } else if (oi->distanceTravelled() > 150) { // TODO: use consts, note: normal distance is about ~300
                    carCount++;
                    obj = objects.erase(obj);
                    newROs++;
                    printf("DISTANCE CAR: %d (%d - %d) speed %f\n", (int)distanceTravelled, oi->getFirstFrame(), oi->getLastBlob().frameNum, oi->getSpeed());
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
